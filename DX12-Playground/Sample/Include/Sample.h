#pragma once

#include <memory>

#include <Game.h>
#include <ECS/Systems/TransformSystem.h>
#include <ECS/Systems/RenderableSystem.h>
#include <ECS/Systems/FrameContext.h>

class Sample : public Game
{
public:
    Sample(const std::wstring& name, int width, int height, bool vSync = false);

    virtual bool LoadContent() override;
    virtual void UnloadContent() override;

protected:
    virtual void OnUpdate(UpdateEventArgs& e) override;
    virtual void OnRender(RenderEventArgs& e) override;

private:
    // Clear helpers (thin wrappers kept in Sample)
    void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                  FLOAT* clearColor);

    void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv,
                    FLOAT depth = 1.0f);

    // Geometry
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    // Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

    // ECS systems
    std::shared_ptr<TransformSystem>  m_TransformSystem;
    std::shared_ptr<RenderableSystem> m_RenderableSystem;

    FrameContext m_FrameCtx;

    // Scene entities
    std::vector<Entity> m_EntityList = std::vector<Entity>(100, INVALID_ENTITY);

};
