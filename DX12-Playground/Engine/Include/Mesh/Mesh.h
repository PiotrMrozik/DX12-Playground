#pragma once

#include <cstdint>

#include <directx/d3d12.h>
#include <wrl.h>

/// GPU-resident mesh: vertex/index buffers, views, and draw metadata.
struct Mesh
{
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW vbView{};
    D3D12_INDEX_BUFFER_VIEW  ibView{};

    uint32_t             indexCount = 0;
    D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
