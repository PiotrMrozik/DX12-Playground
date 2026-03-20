#include <DX12LibPCH.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Game.h>
#include <KeyCodes.h>
#include <Window.h>

Game::Game(const std::wstring& name, int width, int height, bool vSync)
    : m_Name(name)
    , m_Width(width)
    , m_Height(height)
    , m_vSync(vSync)
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_FoV(45.0f)
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
    msaaData.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
    msaaData.SampleCount = m_MSAASampleCount;
    msaaData.Flags       = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    ThrowIfFailed(device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaData, sizeof(msaaData)));

    if (msaaData.NumQualityLevels == 0)
        throw std::exception("MSAA 4x not supported for DXGI_FORMAT_R8G8B8A8_UNORM");

    m_MSAAQualityLevel = msaaData.NumQualityLevels - 1;

    // Create the DSV descriptor heap (one descriptor, allocated once).
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // Create the MSAA RTV descriptor heap (one descriptor, allocated once).
    D3D12_DESCRIPTOR_HEAP_DESC msaaRtvHeapDesc = {};
    msaaRtvHeapDesc.NumDescriptors = 1;
    msaaRtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    msaaRtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&msaaRtvHeapDesc, IID_PPV_ARGS(&m_MSAARTVHeap)));

    return true;
}

void Game::Destroy()
{
    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

void Game::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", fps);
        OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime  = 0.0;
    }
}

void Game::OnRender(RenderEventArgs& e) {}

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

void Game::OnKeyReleased(KeyEventArgs& e)                 {}
void Game::OnMouseMoved(MouseMotionEventArgs& e)           {}
void Game::OnMouseButtonPressed(MouseButtonEventArgs& e)   {}
void Game::OnMouseButtonReleased(MouseButtonEventArgs& e)  {}

void Game::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_FoV -= e.WheelDelta;
    m_FoV  = std::clamp(m_FoV, 12.0f, 90.0f);

    char buffer[256];
    sprintf_s(buffer, "FoV: %f\n", m_FoV);
    OutputDebugStringA(buffer);
}

void Game::OnResize(ResizeEventArgs& e)
{
    m_Width  = e.Width;
    m_Height = e.Height;

    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
        static_cast<float>(m_Width), static_cast<float>(m_Height));

    ResizeMSAAResources(m_Width, m_Height);
}

void Game::OnWindowDestroy()
{
    UnloadContent();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void Game::TransitionResource(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    Microsoft::WRL::ComPtr<ID3D12Resource>             resource,
    D3D12_RESOURCE_STATES beforeState,
    D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(), beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void Game::UpdateBufferResource(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** pDestinationResource,
    ID3D12Resource** pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));

    if (bufferData)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData      = bufferData;
        subresourceData.RowPitch   = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void Game::Present(
    std::shared_ptr<CommandQueue>                      commandQueue,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    UINT currentIndex           = m_pWindow->GetCurrentBackBufferIndex();
    m_FenceValues[currentIndex] = commandQueue->ExecuteCommandList(commandList);
    currentIndex                = m_pWindow->Present();
    commandQueue->WaitForFenceValue(m_FenceValues[currentIndex]);
}

void Game::ResizeMSAAResources(int width, int height)
{
    if (!m_ContentLoaded)
        return;

    Application::Get().Flush();

    width  = std::max(1, width);
    height = std::max(1, height);

    auto device = Application::Get().GetDevice();

    // MSAA colour render target
    {
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format   = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValue.Color[0] = 0.4f;
        clearValue.Color[1] = 0.6f;
        clearValue.Color[2] = 0.9f;
        clearValue.Color[3] = 1.0f;

        auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM, width, height,
            1, 1, m_MSAASampleCount, m_MSAAQualityLevel,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
            &clearValue,
            IID_PPV_ARGS(&m_MSAARenderTarget)));

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

        device->CreateRenderTargetView(m_MSAARenderTarget.Get(), &rtvDesc,
            m_MSAARTVHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // MSAA depth buffer
    {
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format        = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil  = { 1.0f, 0 };

        auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT, width, height,
            1, 1, m_MSAASampleCount, m_MSAAQualityLevel,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_DepthBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format        = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        dsvDesc.Flags         = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc,
            m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}
