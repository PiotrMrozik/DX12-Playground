#pragma once

#include <cstdint>
#include <vector>

#include <directx/d3d12.h>
#include <wrl.h>

#include <Mesh/Mesh.h>
#include <Mesh/MeshData.h>

/// Owns every GPU-resident Mesh in the engine.
/// MeshComponent::meshIndex is an index into this registry.
class MeshRegistry
{
public:
    /// Upload a MeshData to the GPU and return its registry index.
    /// The caller must execute the command list afterwards so that the
    /// upload completes before the intermediate buffers go out of scope.
    /// @param intermediates  Staging buffers are appended here; keep alive
    ///                       until the command list has finished executing.
    uint32_t Add(const MeshData& data,
                 Microsoft::WRL::ComPtr<ID3D12Device2> device,
                 Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                 std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& intermediates);

    const Mesh& Get(uint32_t index) const;
    uint32_t Count() const;

private:
    std::vector<Mesh> m_Meshes;
};
