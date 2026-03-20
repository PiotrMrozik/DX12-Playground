#pragma once
#include <array>
#include <queue>

#include <ECS/Globals.h>

class EntityManager
{
public:
    EntityManager();

    Entity CreateEntity();
    void   DestroyEntity(Entity entity);

    void            SetSignature(Entity entity, EntitySignature signature);
    EntitySignature GetSignature(Entity entity) const;

private:
    std::queue<Entity>                       availableEntities{};
    std::array<EntitySignature, MAX_ENTITIES> signatures{};
    std::uint32_t                            livingEntityCount{};
};
