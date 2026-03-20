#include <ECS/World.h>

Entity World::CreateEntity()
{
    return entityManager.CreateEntity();
}

void World::DestroyEntity(Entity entity)
{
    entityManager.DestroyEntity(entity);
    componentManager.EntityDestroyed(entity);
    systemManager.EntityDestroyed(entity);
}
