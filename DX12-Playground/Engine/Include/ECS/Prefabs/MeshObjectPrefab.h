#pragma once
#include <string>

#include <ECS/Globals.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MeshComponent.h>
#include <ECS/Components/TagComponent.h>

class World;

struct MeshObjectPrefab
{
    std::string        tag      { "MeshObject" };
    TransformComponent transform{};
    MeshComponent      mesh{};

    Entity Spawn(World& world) const;
};
