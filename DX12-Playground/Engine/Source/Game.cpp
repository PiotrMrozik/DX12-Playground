#include <DX12LibPCH.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Game.h>
#include <KeyCodes.h>
#include <Window.h>
#include <Camera/OrbitCamera.h>

#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

// ---------------------------------------------------------------------------
// ImGuiSrvDescAllocator
// ---------------------------------------------------------------------------

void Game::ImGuiSrvDescAllocator::Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
    HeapStartCpu = heap->GetCPUDescriptorHandleForHeapStart();
    HeapStartGpu = heap->GetGPUDescriptorHandleForHeapStart();
    Increment    = device->GetDescriptorHandleIncrementSize(desc.Type);
    FreeIndices.reserve(static_cast<int>(desc.NumDescriptors));
    for (int n = static_cast<int>(desc.NumDescriptors); n > 0; --n)
        FreeIndices.push_back(n - 1);
}

void Game::ImGuiSrvDescAllocator::Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
{
    assert(!FreeIndices.empty() && "ImGui SRV descriptor heap exhausted — increase heap size.");
    int idx = FreeIndices.back();
    FreeIndices.pop_back();
    outCpu->ptr = HeapStartCpu.ptr + static_cast<SIZE_T>(idx) * Increment;
    outGpu->ptr = HeapStartGpu.ptr + static_cast<UINT64>(idx) * Increment;
}

void Game::ImGuiSrvDescAllocator::Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE /*gpu*/)
{
    int idx = static_cast<int>((cpu.ptr - HeapStartCpu.ptr) / Increment);
    FreeIndices.push_back(idx);
}

// ---------------------------------------------------------------------------

Game::Game(const std::wstring& name, int width, int height, bool vSync)
    : m_Name(name)
    , m_Width(width)
    , m_Height(height)
    , m_vSync(vSync)
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_FoV(60.0f)
    , m_ContentLoaded(false)
{
}

Game::~Game()
{
    assert(!m_pWindow && "Use Game::Destroy() before destruction.");
}

bool Game::Initialize()
{
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
    m_pWindow->RegisterCallbacks(shared_from_this());
    m_pWindow->Show();

    auto device = Application::Get().GetDevice();

    // Query MSAA quality levels for the back-buffer format.
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaData = {};
    msaaData.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    msaaData.SampleCount = m_MSAASampleCount;
    msaaData.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaData, sizeof(msaaData)));

    if (msaaData.NumQualityLevels == 0)
        throw std::exception("MSAA 4x not supported for DXGI_FORMAT_R8G8B8A8_UNORM");

    m_MSAAQualityLevel = msaaData.NumQualityLevels - 1;

    // Create the DSV descriptor heap (one descriptor, allocated once).
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // Create the MSAA RTV descriptor heap (one descriptor, allocated once).
    D3D12_DESCRIPTOR_HEAP_DESC msaaRtvHeapDesc = {};
    msaaRtvHeapDesc.NumDescriptors = 1;
    msaaRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    msaaRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&msaaRtvHeapDesc, IID_PPV_ARGS(&m_MSAARTVHeap)));

    // --- ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    // Shader-visible SRV heap: enough descriptors for ImGui's dynamic atlas.
    D3D12_DESCRIPTOR_HEAP_DESC imguiSrvDesc = {};
    imguiSrvDesc.NumDescriptors = 64;
    imguiSrvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    imguiSrvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&imguiSrvDesc, IID_PPV_ARGS(&m_ImGuiSrvHeap)));
    m_ImGuiSrvDescAllocator.Create(device.Get(), m_ImGuiSrvHeap.Get());

    ImGui_ImplWin32_Init(m_pWindow->GetWindowHandle());

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device           = device.Get();
    initInfo.CommandQueue     = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetD3D12CommandQueue().Get();
    initInfo.NumFramesInFlight = Window::BufferCount;
    initInfo.RTVFormat        = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat        = DXGI_FORMAT_UNKNOWN;
    initInfo.SrvDescriptorHeap = m_ImGuiSrvHeap.Get();
    initInfo.UserData         = this;
    initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
        static_cast<Game*>(info->UserData)->m_ImGuiSrvDescAllocator.Alloc(outCpu, outGpu);
    };
    initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
        static_cast<Game*>(info->UserData)->m_ImGuiSrvDescAllocator.Free(cpu, gpu);
    };
    ImGui_ImplDX12_Init(&initInfo);

    return true;
}

void Game::Destroy()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

void Game::OnUpdate(UpdateEventArgs& e)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    OnImGui();
}

void Game::OnRender(RenderEventArgs& e)
{
}

void Game::RenderImGui(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    ImGui::Render();
    ID3D12DescriptorHeap* heaps[] = {m_ImGuiSrvHeap.Get()};
    commandList->SetDescriptorHeaps(1, heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}

void Game::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
        case KeyCode::F11:
            m_pWindow->ToggleFullscreen();
            break;
        }
    case KeyCode::V:
        m_pWindow->ToggleVSync();
        break;
    }
}

void Game::OnKeyReleased(KeyEventArgs& e)
{
}
void Game::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (e.LeftButton)
    {
        m_Camera.yaw += e.RelX * m_Camera.rotationSensitivity;
        m_Camera.pitch -= e.RelY * m_Camera.rotationSensitivity;
        m_Camera.pitch  = std::clamp(m_Camera.pitch,
                                     OrbitCamera::MinPitch,
                                     OrbitCamera::MaxPitch);
    }

    if (e.RightButton)
    {
        m_Camera.Pan(static_cast<float>(e.RelX * m_Camera.panSensitivity),
                     static_cast<float>(e.RelY * m_Camera.panSensitivity));
    }
}
void Game::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
}
void Game::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
}
void Game::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Camera.radius -= e.WheelDelta * m_Camera.zoomSpeed;
    m_Camera.radius  = std::max(m_Camera.radius, 0.5f);
}
void Game::OnResize(ResizeEventArgs& e)
{
    m_Width = e.Width;
    m_Height = e.Height;

    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height));

    ResizeMSAAResources(m_Width, m_Height);
}

void Game::OnWindowDestroy()
{
    UnloadContent();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void Game::TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                              Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState,
                              D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void Game::UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                                ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
                                size_t numElements, size_t elementSize, const void* bufferData,
                                D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    ThrowIfFailed(
        device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags), D3D12_RESOURCE_STATE_COMMON,
                                        nullptr, IID_PPV_ARGS(pDestinationResource)));

    if (bufferData)
    {
        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                      D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                                                      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                      IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
    }
}

void Game::Present(std::shared_ptr<CommandQueue> commandQueue,
                   Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    UINT currentIndex = m_pWindow->GetCurrentBackBufferIndex();
    m_FenceValues[currentIndex] = commandQueue->ExecuteCommandList(commandList);
    currentIndex = m_pWindow->Present();
    commandQueue->WaitForFenceValue(m_FenceValues[currentIndex]);
}

void Game::ResizeMSAAResources(int width, int height)
{
    if (!m_ContentLoaded)
        return;

    Application::Get().Flush();

    width = std::max(1, width);
    height = std::max(1, height);

    auto device = Application::Get().GetDevice();

    // MSAA colour render target
    {
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValue.Color[0] = 0.4f;
        clearValue.Color[1] = 0.6f;
        clearValue.Color[2] = 0.9f;
        clearValue.Color[3] = 1.0f;

        auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, m_MSAASampleCount,
                                                 m_MSAAQualityLevel, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                      D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                                                      &clearValue, IID_PPV_ARGS(&m_MSAARenderTarget)));

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

        device->CreateRenderTargetView(m_MSAARenderTarget.Get(), &rtvDesc,
                                       m_MSAARTVHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // MSAA depth buffer
    {
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil = {1.0f, 0};

        auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 1, m_MSAASampleCount,
                                                 m_MSAAQualityLevel, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                      D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                      &clearValue, IID_PPV_ARGS(&m_DepthBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc, m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}
