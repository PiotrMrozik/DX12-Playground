#pragma once
#include <ECS/System/System.h>
#include <ECS/World.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/DirectionalLightComponent.h>
#include <ECS/Components/PointLightComponent.h>
#include <ECS/Systems/FrameContext.h>

// -------------------------------------------------------------------------------------------------
// Why two systems instead of one?
//
// ECS signatures are AND-masks: a system matches entities that have ALL required components.
// There is no built-in OR query.  Directional lights don't need a position (direction is
// self-contained in DirectionalLightComponent), while point lights do (they use TransformComponent).
// Forcing both into one signature would require every light entity to have both component types.
//
// The canonical ECS solution is one system per archetype.  Both systems write into the same
// FrameContext so the renderer still sees a unified light list.
// -------------------------------------------------------------------------------------------------

// Required signature: DirectionalLightComponent
class DirectionalLightSystem : public System
{
public:
    void Update(World& world, FrameContext& ctx)
    {
        for (Entity e : entities)
        {
            const auto& lc = world.GetComponent<DirectionalLightComponent>(e);
            ctx.directionalLights.push_back({ lc.direction, lc.intensity, lc.color });
        }
    }
};

// Required signature: TransformComponent + PointLightComponent
// Position is read from TransformComponent so there is no redundant field on the component.
class PointLightSystem : public System
{
public:
    void Update(World& world, FrameContext& ctx)
    {
        for (Entity e : entities)
        {
            const auto& tc = world.GetComponent<TransformComponent>(e);
            const auto& lc = world.GetComponent<PointLightComponent>(e);
            ctx.pointLights.push_back({ tc.position, lc.radius, lc.color, lc.intensity });
        }
    }
};
