#pragma once
#include <memory>

#include <ECS/Entity/EntityManager.h>
#include <ECS/Component/ComponentManager.h>
#include <ECS/System/SystemManager.h>

// World is the public facade over all three ECS managers.
// User code should only talk to World, never to the managers directly.
class World
{
public:
    Entity CreateEntity();
    void   DestroyEntity(Entity entity);

    // --- Component registration & access ---

    template <typename T>
    void RegisterComponent()
    {
        componentManager.RegisterComponent<T>();
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        componentManager.AddComponent<T>(entity, std::move(component));

        auto sig = entityManager.GetSignature(entity);
        sig.set(componentManager.GetComponentType<T>(), true);
        entityManager.SetSignature(entity, sig);

        systemManager.EntitySignatureChanged(entity, sig);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        componentManager.RemoveComponent<T>(entity);

        auto sig = entityManager.GetSignature(entity);
        sig.set(componentManager.GetComponentType<T>(), false);
        entityManager.SetSignature(entity, sig);

        systemManager.EntitySignatureChanged(entity, sig);
    }

    template <typename T>
    T& GetComponent(Entity entity)
    {
        return componentManager.GetComponent<T>(entity);
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        return componentManager.GetComponentType<T>();
    }

    // --- System registration ---

    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return systemManager.RegisterSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(EntitySignature signature)
    {
        systemManager.SetSignature<T>(signature);
    }

private:
    EntityManager    entityManager{};
    ComponentManager componentManager{};
    SystemManager    systemManager{};
};
