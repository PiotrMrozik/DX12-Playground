#include <ECS/Prefabs/PointLightPrefab.h>
#include <ECS/World.h>

Entity PointLightPrefab::Spawn(World& world) const
{
    Entity e = world.CreateEntity();
    world.AddComponent<TransformComponent> (e, transform);
    world.AddComponent<PointLightComponent>(e, light);
    world.AddComponent<TagComponent>       (e, TagComponent(tag));
    return e;
}
