#pragma once

#include <cstdint>
#include <vector>

#include <directx/d3d12.h>

#include <Mesh/Vertex.h>

/// CPU-side geometry: vertices, indices, and primitive topology.
/// Feed this to MeshRegistry to upload to the GPU.
struct MeshData
{
    std::vector<VertexNormal> vertices;
    std::vector<uint16_t>     indices;
    D3D_PRIMITIVE_TOPOLOGY    topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
