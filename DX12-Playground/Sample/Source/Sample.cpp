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
#include <ECS/Systems/PathFollowerSystem.h>
#include <ECS/Systems/FrameContext.h>
#include <ECS/Prefabs/MeshObjectPrefab.h>
#include <ECS/Prefabs/PointLightPrefab.h>
#include <ECS/Components/PathFollowerComponent.h>
#include <ECS/Components/PointLightComponent.h>
#include <Math/CirclePath.h>

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

static constexpr uint32_t MAX_POINT_LIGHTS = 8; // must match shader #define

struct PointLightCBuffer
{
    uint32_t      count;
    float         _pad[3];
    PointLightData lights[MAX_POINT_LIGHTS];
};

static constexpr UINT PointLightCBSize =
    (sizeof(PointLightCBuffer) + 255u) & ~255u; // round up to 256-byte alignment

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
    uint32_t cubeIndex = m_MeshRegistry.Add(Primitives::CreateTorusKnot(), device, commandList, intermediates);

    // Load shaders.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    // Vertex input layout - matches VertexNormal (Position + Normal).
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

    CD3DX12_ROOT_PARAMETER1 rootParameters[4];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b0: World
    rootParameters[1].InitAsConstants(sizeof(XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b1: VP
    rootParameters[2].InitAsConstants(sizeof(XMFLOAT4) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b0: CameraPos (PS)
    rootParameters[3].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                               D3D12_SHADER_VISIBILITY_PIXEL);                      // b1: PointLightBuffer (PS)

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
    m_World.RegisterComponent<PathFollowerComponent>();
    m_World.RegisterComponent<PointLightComponent>();

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

    m_PathFollowerSystem = m_World.RegisterSystem<PathFollowerSystem>();
    {
        EntitySignature sig;
        sig.set(m_World.GetComponentType<TransformComponent>());
        sig.set(m_World.GetComponentType<PathFollowerComponent>());
        m_World.SetSystemSignature<PathFollowerSystem>(sig);
    }

    m_PointLightSystem = m_World.RegisterSystem<PointLightSystem>();
    {
        EntitySignature sig;
        sig.set(m_World.GetComponentType<TransformComponent>());
        sig.set(m_World.GetComponentType<PointLightComponent>());
        m_World.SetSystemSignature<PointLightSystem>(sig);
    }

    // --- ECS: create torus (static at origin) ---
    MeshObjectPrefab{
        "TorusKnot",
        TransformComponent{ {0.f, 0.f, 0.f}, {}, {3.f, 3.f, 3.f} },
        MeshComponent(cubeIndex)
    }.Spawn(m_World);

    // --- Point lights - each on its own orbit around the scene origin ---
    {
        struct LightDesc { XMFLOAT3 color; float orbitRadius; float orbitHeight; float speed; float startT; XMFLOAT3 normal; };
        const LightDesc lights[] = {
            { { 1.0f, 0.5f, 0.3f }, 6.f, 3.f, 4.0f, 0.00f, {  0.3f, 1.0f,  0.2f } }, // warm orange - slight forward tilt
            { { 0.3f, 0.6f, 1.0f }, 9.f, 5.f, 4.6f, 0.25f, { -0.5f, 0.8f,  0.4f } }, // cool blue   - left-leaning
            { { 0.4f, 1.0f, 0.5f }, 7.f, 2.f, 8.0f, 0.50f, {  0.6f, 0.6f, -0.5f } }, // green       - steep diagonal
            { { 1.0f, 0.3f, 0.8f }, 5.f, 6.f, 6.0f, 0.75f, {  0.1f, 0.5f,  0.9f } }, // pink        - near-horizontal
        };
        for (const auto& ld : lights)
        {
            PointLightPrefab prefab;
            prefab.light.color     = ld.color;
            prefab.light.intensity = 2.0f;
            prefab.light.radius    = 15.f;
            Entity e = prefab.Spawn(m_World);

            PathFollowerComponent pf;
            pf.path      = std::make_shared<Math::CirclePath>(
                               DirectX::XMFLOAT3{0.f, ld.orbitHeight, 0.f}, ld.orbitRadius, ld.normal);
            pf.speed     = ld.speed;
            pf.t         = ld.startT;
            pf.loop      = true;
            pf.playing   = true;
            pf.arcLength = -1.f;
            m_World.AddComponent(e, std::move(pf));
        }
    }

    // --- Camera ---
    m_Camera.target = { 0.f, 0.f, 0.f };
    m_Camera.radius = 10.f;
    m_Camera.fovY   = XMConvertToRadians(m_FoV);

    // --- UI: register component inspectors ---
    m_InspectorRegistry.Register<TransformComponent>(m_World, "Transform");
    m_InspectorRegistry.Register<TagComponent>(m_World, "Tag");
    m_InspectorRegistry.Register<MeshComponent>(m_World, "Mesh");
    m_InspectorRegistry.Register<PathFollowerComponent>(m_World, "Path Follower");

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

    // --- Point light constant buffer (upload heap, persistently mapped) ---
    {
        auto heapProps  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PointLightCBSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_PointLightCB)));

        m_PointLightCB->Map(0, nullptr, &m_PointLightCBMapped);
    }

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

    // Sync FoV (may have changed via mouse wheel).
    m_Camera.fovY = XMConvertToRadians(m_FoV);

    // Run ECS systems.
    const float dt = static_cast<float>(e.ElapsedTime);
    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    m_FrameCtx.Reset();
    m_FrameCtx.viewProj = m_Camera.GetViewProjMatrix(aspectRatio);
    auto camPos = m_Camera.GetPosition();
    m_FrameCtx.cameraPos = { camPos.x, camPos.y, camPos.z, 1.f };
    m_PathFollowerSystem->Update(m_World, dt);
    m_TransformSystem->Update(m_World);
    m_RenderableSystem->Update(m_World, m_FrameCtx);
    m_PointLightSystem->Update(m_World, m_FrameCtx);
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

    // Upload point light data and bind root CBV (slot 3 → PS b1)
    {
        PointLightCBuffer lightCB{};
        lightCB.count = static_cast<uint32_t>(
            std::min(m_FrameCtx.pointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS)));
        std::memcpy(lightCB.lights, m_FrameCtx.pointLights.data(),
                    lightCB.count * sizeof(PointLightData));
        std::memcpy(m_PointLightCBMapped, &lightCB, sizeof(PointLightCBuffer));
    }
    commandList->SetGraphicsRootConstantBufferView(3, m_PointLightCB->GetGPUVirtualAddress());

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
