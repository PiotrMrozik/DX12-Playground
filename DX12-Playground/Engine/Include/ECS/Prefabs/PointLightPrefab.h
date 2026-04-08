#pragma once
#include <string>

#include <ECS/Globals.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/PointLightComponent.h>
#include <ECS/Components/TagComponent.h>

class World;

struct PointLightPrefab
{
    std::string         tag      { "PointLight" };
    TransformComponent  transform{};
    PointLightComponent light    {};

    Entity Spawn(World& world) const;
};
