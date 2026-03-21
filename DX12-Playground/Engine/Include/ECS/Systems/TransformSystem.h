#pragma once
#include <ECS/System/System.h>
#include <ECS/World.h>
#include <ECS/Components/TransformComponent.h>

// Required signature: TransformComponent
//
// Computes and caches the world matrix for every entity with a TransformComponent.
// Must run first in the frame pipeline so downstream systems (Camera, Renderable)
// can read TransformComponent::worldMatrix without recomputing it.
class TransformSystem : public System
{
public:
    void Update(World& world)
    {
        for (Entity e : entities)
        {
            auto& tc      = world.GetComponent<TransformComponent>(e);
            tc.worldMatrix = tc.GetWorldMatrix();
        }
    }
};
