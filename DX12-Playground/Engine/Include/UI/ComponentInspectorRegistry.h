#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include <ECS/Globals.h>
#include <ECS/World.h>
#include <UI/ComponentInspectors.h>

class ComponentInspectorRegistry
{
public:
    using InspectFn = std::function<void(World&, Entity)>;

    struct Entry
    {
        std::string name;
        InspectFn   inspect;
    };

    // Register an inspector for component T.
    // Call after World::RegisterComponent<T>().
    template <typename T>
    void Register(World& world, const std::string& name)
    {
        ComponentType type = world.GetComponentType<T>();
        m_Inspectors[type] = Entry{
            name,
            [](World& w, Entity e) {
                InspectComponent(w.GetComponent<T>(e));
            }
        };
    }

    // Draw all registered components the entity has.
    void InspectEntity(World& world, Entity entity) const;

private:
    std::unordered_map<ComponentType, Entry> m_Inspectors;
};
