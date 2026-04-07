#include <DX12LibPCH.h>

#include <Mesh/MeshRegistry.h>
#include <Helpers.h>

#include <directx/d3dx12.h>

uint32_t MeshRegistry::Add(const MeshData& data,
                           Microsoft::WRL::ComPtr<ID3D12Device2> device,
                           Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                           std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& intermediates)
{
    Mesh mesh;
    mesh.indexCount = static_cast<uint32_t>(data.indices.size());
    mesh.topology   = data.topology;

    // --- Vertex buffer upload ---------------------------------------------------
    {
        const size_t bufferSize = data.vertices.size() * sizeof(VertexNormal);

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&mesh.vertexBuffer)));

        ComPtr<ID3D12Resource> staging;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&staging)));

        D3D12_SUBRESOURCE_DATA sub{};
        sub.pData      = data.vertices.data();
        sub.RowPitch   = bufferSize;
        sub.SlicePitch = bufferSize;
        UpdateSubresources(commandList.Get(), mesh.vertexBuffer.Get(), staging.Get(), 0, 0, 1, &sub);

        intermediates.push_back(std::move(staging));

        mesh.vbView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
        mesh.vbView.SizeInBytes    = static_cast<UINT>(bufferSize);
        mesh.vbView.StrideInBytes  = sizeof(VertexNormal);
    }

    // --- Index buffer upload ----------------------------------------------------
    {
        const size_t bufferSize = data.indices.size() * sizeof(uint16_t);

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&mesh.indexBuffer)));

        ComPtr<ID3D12Resource> staging;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&staging)));

        D3D12_SUBRESOURCE_DATA sub{};
        sub.pData      = data.indices.data();
        sub.RowPitch   = bufferSize;
        sub.SlicePitch = bufferSize;
        UpdateSubresources(commandList.Get(), mesh.indexBuffer.Get(), staging.Get(), 0, 0, 1, &sub);

        intermediates.push_back(std::move(staging));

        mesh.ibView.BufferLocation = mesh.indexBuffer->GetGPUVirtualAddress();
        mesh.ibView.Format         = DXGI_FORMAT_R16_UINT;
        mesh.ibView.SizeInBytes    = static_cast<UINT>(bufferSize);
    }

    uint32_t index = static_cast<uint32_t>(m_Meshes.size());
    m_Meshes.push_back(std::move(mesh));
    return index;
}

const Mesh& MeshRegistry::Get(uint32_t index) const
{
    return m_Meshes[index];
}

uint32_t MeshRegistry::Count() const
{
    return static_cast<uint32_t>(m_Meshes.size());
}
