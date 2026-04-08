#include <ECS/Entity/EntityManager.h>

#include <cassert>
#include <stdexcept>

EntityManager::EntityManager()
{
    for (Entity id = 0; id < MAX_ENTITIES; ++id)
        availableEntities.push(id);
}

Entity EntityManager::CreateEntity()
{
    assert(livingEntityCount < MAX_ENTITIES && "Max entity count reached.");
    if (livingEntityCount >= MAX_ENTITIES)
        return INVALID_ENTITY;

    const Entity id = availableEntities.front();
    availableEntities.pop();
    ++livingEntityCount;
    livingEntities.insert(id);
    return id;
}

void EntityManager::DestroyEntity(Entity entity)
{
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    signatures[entity].reset();
    availableEntities.push(entity);
    --livingEntityCount;
    livingEntities.erase(entity);
}

void EntityManager::SetSignature(Entity entity, EntitySignature signature)
{
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    signatures[entity] = signature;
}

EntitySignature EntityManager::GetSignature(Entity entity) const
{
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    return signatures[entity];
}
