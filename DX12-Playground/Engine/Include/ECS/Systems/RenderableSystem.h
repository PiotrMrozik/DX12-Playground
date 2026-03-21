#pragma once
#include <ECS/System/System.h>
#include <ECS/World.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MeshComponent.h>
#include <ECS/Systems/FrameContext.h>

// Required signature: TransformComponent + MeshComponent
//
// Collects all renderable entities into FrameContext::renderList each frame.
// Must run after TransformSystem so TransformComponent::worldMatrix is current.
class RenderableSystem : public System
{
public:
    void Update(World& world, FrameContext& ctx)
    {
        for (Entity e : entities)
        {
            const auto& tc = world.GetComponent<TransformComponent>(e);
            const auto& mc = world.GetComponent<MeshComponent>(e);
            ctx.renderList.push_back({ e, mc.meshIndex, tc.worldMatrix });
        }
    }
};
