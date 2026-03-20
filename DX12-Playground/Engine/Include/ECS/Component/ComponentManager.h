#pragma once
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include <ECS/Component/ComponentArray.h>

class ComponentManager
{
public:
    template <typename T>
    void RegisterComponent()
    {
        const auto key = std::type_index(typeid(T));
        assert(componentTypes.find(key) == componentTypes.end() && "Component type already registered.");

        componentTypes[key]  = nextComponentType;
        componentArrays[key] = std::make_shared<ComponentArray<T>>();
        ++nextComponentType;
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        const auto key = std::type_index(typeid(T));
        assert(componentTypes.find(key) != componentTypes.end() && "Component type not registered.");
        return componentTypes[key];
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        GetComponentArray<T>()->InsertData(entity, std::move(component));
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template <typename T>
    T& GetComponent(Entity entity)
    {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(Entity entity);

private:
    std::unordered_map<std::type_index, ComponentType>                    componentTypes{};
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays{};
    ComponentType nextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        const auto key = std::type_index(typeid(T));
        assert(componentTypes.find(key) != componentTypes.end() && "Component type not registered.");
        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[key]);
    }
};
