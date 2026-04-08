#pragma once

#include <memory>

#include <Game.h>
#include <Mesh/MeshRegistry.h>
#include <ECS/Systems/TransformSystem.h>
#include <ECS/Systems/RenderableSystem.h>
#include <ECS/Systems/PathFollowerSystem.h>
#include <ECS/Systems/FrameContext.h>
#include <UI/ComponentInspectorRegistry.h>

class Sample : public Game
{
public:
    Sample(const std::wstring& name, int width, int height, bool vSync = true);

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
    MeshRegistry m_MeshRegistry;

    // Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

    // ECS systems
    std::shared_ptr<TransformSystem>     m_TransformSystem;
    std::shared_ptr<RenderableSystem>    m_RenderableSystem;
    std::shared_ptr<PathFollowerSystem>  m_PathFollowerSystem;

    FrameContext m_FrameCtx;

    ComponentInspectorRegistry m_InspectorRegistry;
};
