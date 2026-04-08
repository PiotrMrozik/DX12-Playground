#include <DX12LibPCH.h>
#include <UI/ComponentInspectorRegistry.h>

void ComponentInspectorRegistry::InspectEntity(World& world, Entity entity) const
{
    for (const auto& [type, entry] : m_Inspectors)
    {
        if (world.HasComponent(entity, type))
            entry.inspect(world, entity);
    }
}
