#include <Sample.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Helpers.h>
#include <Window.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <directx/d3dx12.h>
#include <d3dcompiler.h>

#include <Mesh/Vertex.h>
#include <Mesh/Primitives.h>
#include <Mesh/MeshRegistry.h>

#include <ECS/Systems/TransformSystem.h>
#include <ECS/Systems/RenderableSystem.h>
#include <ECS/Systems/FrameContext.h>
#include <ECS/Prefabs/MeshObjectPrefab.h>

#include <UI/StatsPanel.h>
#include <UI/SceneHierarchyPanel.h>
#include <UI/InspectorPanel.h>

#include <algorithm>
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

using namespace DirectX;

Sample::Sample(const std::wstring& name, int width, int height, bool vSync)
    : Game(name, width, height, vSync)
{
}

bool Sample::LoadContent()
{
    auto device = Application::Get().GetDevice();
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->GetCommandList();

    // Upload primitive meshes via the registry.
    std::vector<ComPtr<ID3D12Resource>> intermediates;
    uint32_t cubeIndex = m_MeshRegistry.Add(Primitives::CreateTorus(), device, commandList, intermediates);

    // Load shaders.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    // Vertex input layout — matches VertexNormal (Position + Normal).
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_ROOT_PARAMETER1 rootParameters[3];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b0: World
    rootParameters[1].InitAsConstants(sizeof(XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b1: VP
    rootParameters[2].InitAsConstants(sizeof(XMFLOAT4) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b0: CameraPos (PS)

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion,
                                                        &rootSignatureBlob, &errorBlob));

    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
                                              rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

    // Pipeline state object.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = {inputLayout, _countof(inputLayout)};
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;
    pipelineStateStream.SampleDesc = {m_MSAASampleCount, m_MSAAQualityLevel};

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {sizeof(PipelineStateStream), &pipelineStateStream};
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    // --- ECS: register components ---
    m_World.RegisterComponent<TransformComponent>();
    m_World.RegisterComponent<TagComponent>();
    m_World.RegisterComponent<MeshComponent>();

    // --- ECS: register systems ---
    m_TransformSystem = m_World.RegisterSystem<TransformSystem>();
    {
        EntitySignature sig;
        sig.set(m_World.GetComponentType<TransformComponent>());
        m_World.SetSystemSignature<TransformSystem>(sig);
    }

    m_RenderableSystem = m_World.RegisterSystem<RenderableSystem>();
    {
        EntitySignature sig;
        sig.set(m_World.GetComponentType<TransformComponent>());
        sig.set(m_World.GetComponentType<MeshComponent>());
        m_World.SetSystemSignature<RenderableSystem>(sig);
    }

    // --- ECS: create cube entities ---
    for (int i = 0; i < 1; ++i)
    {
        float px    = static_cast<float>(rand() % 10 - 5);
        float py    = static_cast<float>(rand() % 10 - 5);
        float pz    = static_cast<float>(rand() % 10 - 5);
        float scale = static_cast<float>(rand() % 1) + 0.5f;

        MeshObjectPrefab{
            "Cube",
           // TransformComponent{ {px, py, pz}, {}, {scale, scale, scale} },
            TransformComponent{ {0, 0, 0}, {}, {3, 3, 3} },
            MeshComponent(cubeIndex)
        }.Spawn(m_World);
    }

    // --- Camera ---
    m_Camera.target = { 0.f, 0.f, 0.f };
    m_Camera.radius = 10.f;
    m_Camera.fovY   = XMConvertToRadians(m_FoV);

    // --- UI: register component inspectors ---
    m_InspectorRegistry.Register<TransformComponent>(m_World, "Transform");
    m_InspectorRegistry.Register<TagComponent>(m_World, "Tag");
    m_InspectorRegistry.Register<MeshComponent>(m_World, "Mesh");

    // --- UI: build panel layout ---
    auto& ui = GetUILayer();

    ui.AddPanel(std::make_unique<StatsPanel>());

    auto hierarchy = std::make_unique<SceneHierarchyPanel>(m_World);
    auto* hierarchyPtr = hierarchy.get();
    ui.AddPanel(std::move(hierarchy));

    ui.AddPanel(std::make_unique<InspectorPanel>(*hierarchyPtr, m_InspectorRegistry, m_World));

    ui.AddCustomPanel("Camera", [this]() {
        ImGui::Begin("Camera");
        ImGui::SliderFloat("Radius", &m_Camera.radius, 1.0f, 50.0f);
        ImGui::SliderFloat("FoV",    &m_FoV,           12.0f, 90.0f);
        ImGui::End();
    });

    m_ContentLoaded = true;

    ResizeMSAAResources(GetClientWidth(), GetClientHeight());

    return true;
}

void Sample::UnloadContent()
{
    m_ContentLoaded = false;
}


void Sample::OnUpdate(UpdateEventArgs& e)
{
    Game::OnUpdate(e); // FPS counter

    for (Entity entity : m_TransformSystem->entities)
    {
        auto& tc = m_World.GetComponent<TransformComponent>(entity);
       // tc.rotation.y += XMConvertToRadians(static_cast<float>(e.ElapsedTime * 90.0 ));
        tc.rotation.x += XMConvertToRadians(static_cast<float>(e.ElapsedTime * 45.0 ));
        //tc.rotation.z += XMConvertToRadians(static_cast<float>(e.ElapsedTime * 30.0 ));
    }

    // Sync FoV (may have changed via mouse wheel).
    m_Camera.fovY = XMConvertToRadians(m_FoV);

    // Run ECS systems.
    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    m_FrameCtx.Reset();
    m_FrameCtx.viewProj = m_Camera.GetViewProjMatrix(aspectRatio);
    auto camPos = m_Camera.GetPosition();
    m_FrameCtx.cameraPos = { camPos.x, camPos.y, camPos.z, 1.f };
    m_TransformSystem->Update(m_World);
    m_RenderableSystem->Update(m_World, m_FrameCtx);
}

void Sample::ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                      FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Sample::ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv,
                        FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void Sample::OnRender(RenderEventArgs& e)
{
    Game::OnRender(e);

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto msaaRtv = m_MSAARTVHeap->GetCPUDescriptorHandleForHeapStart();
    auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Phase 1: MSAA RT RESOLVE_SOURCE -> RENDER_TARGET
    TransitionResource(commandList, m_MSAARenderTarget, D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                       D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Phase 2: Clear and draw to MSAA targets
    {
        FLOAT clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};
        ClearRTV(commandList, msaaRtv, clearColor);
        ClearDepth(commandList, dsv);
    }

    commandList->SetPipelineState(m_PipelineState.Get());
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    commandList->OMSetRenderTargets(1, &msaaRtv, FALSE, &dsv);

    commandList->SetGraphicsRoot32BitConstants(1, sizeof(XMMATRIX) / 4, &m_FrameCtx.viewProj, 0);
    commandList->SetGraphicsRoot32BitConstants(2, sizeof(XMFLOAT4) / 4, &m_FrameCtx.cameraPos, 0);

    for (const auto& entry : m_FrameCtx.renderList)
    {
        const Mesh& mesh = m_MeshRegistry.Get(entry.meshIndex);

        commandList->IASetPrimitiveTopology(mesh.topology);
        commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
        commandList->IASetIndexBuffer(&mesh.ibView);

        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &entry.worldMatrix, 0);
        commandList->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
    }

    // Phase 3: Transition both resources for resolve
    TransitionResource(commandList, m_MSAARenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET,
                       D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST);

    // Phase 4: Resolve MSAA -> swap chain back buffer
    commandList->ResolveSubresource(backBuffer.Get(), 0, m_MSAARenderTarget.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Phase 5: back buffer RESOLVE_DEST -> RENDER_TARGET for ImGui overlay
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RESOLVE_DEST,
                       D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto backBufferRtv = m_pWindow->GetCurrentRenderTargetView();
    commandList->OMSetRenderTargets(1, &backBufferRtv, FALSE, nullptr);
    RenderImGui(commandList);

    // Phase 6: back buffer RENDER_TARGET -> PRESENT, then present
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    Present(commandQueue, commandList);
}
