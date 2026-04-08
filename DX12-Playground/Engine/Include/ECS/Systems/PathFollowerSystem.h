#pragma once
#include <ECS/System/System.h>
#include <ECS/World.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/PathFollowerComponent.h>

// Required signature: TransformComponent + PathFollowerComponent
//
// Each frame:
//  1. Lazily computes and caches the path arc length.
//  2. If playing, advances t by (speed / arcLength) * dt.
//  3. Evaluates the path at t and writes the result to TransformComponent::position.
//  4. If orientToPath is set, aligns yaw (rotation.y) to the path tangent.
//
// Must run after TransformSystem so its worldMatrix write does not clobber
// the position written here.
class PathFollowerSystem : public System
{
public:
    void Update(World& world, float dt);
};
