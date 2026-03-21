#include <ECS/Prefabs/MeshObjectPrefab.h>
#include <ECS/World.h>

Entity MeshObjectPrefab::Spawn(World& world) const
{
    Entity e = world.CreateEntity();
    world.AddComponent<TransformComponent>(e, transform);
    world.AddComponent<MeshComponent>     (e, mesh);
    world.AddComponent<TagComponent>      (e, TagComponent(tag));
    return e;
}
