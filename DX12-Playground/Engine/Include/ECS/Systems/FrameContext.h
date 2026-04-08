#pragma once
#include <vector>
#include <DirectXMath.h>

#include <ECS/Globals.h>

// Flattened CPU-side light descriptors, ready to be uploaded to a constant buffer.
struct DirectionalLightData
{
    DirectX::XMFLOAT3 direction;
    float             intensity;
    DirectX::XMFLOAT3 color;
    float             _pad{ 0.f }; // keeps struct 32-byte aligned
};

struct PointLightData
{
    DirectX::XMFLOAT3 position;
    float             radius;
    DirectX::XMFLOAT3 color;
    float             intensity;
};

// One entry per renderable entity, consumed by the renderer each frame.
struct RenderEntry
{
    Entity            entity;
    std::uint32_t     meshIndex;
    DirectX::XMMATRIX worldMatrix;
};

// Aggregated per-frame data produced by systems and consumed by the renderer.
// World/Scene owns one instance; systems write into it, renderer reads from it.
struct FrameContext
{
    DirectX::XMMATRIX                viewProj{ DirectX::XMMatrixIdentity() };
    DirectX::XMFLOAT4                cameraPos{ 0.f, 0.f, 0.f, 1.f };
    std::vector<DirectionalLightData> directionalLights;
    std::vector<PointLightData>       pointLights;
    std::vector<RenderEntry>          renderList;

    // Call at the start of each frame before running the systems.
    void Reset()
    {
        viewProj = DirectX::XMMatrixIdentity();
        cameraPos = { 0.f, 0.f, 0.f, 1.f };
        directionalLights.clear();
        pointLights.clear();
        renderList.clear();
    }
};
