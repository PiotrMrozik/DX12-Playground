#pragma once
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include <ECS/System/System.h>

class SystemManager
{
public:
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const auto key = std::type_index(typeid(T));
        assert(systems.find(key) == systems.end() && "System already registered.");

        auto system  = std::make_shared<T>();
        systems[key] = system;
        return system;
    }

    template <typename T>
    void SetSignature(EntitySignature signature)
    {
        const auto key = std::type_index(typeid(T));
        assert(systems.find(key) != systems.end() && "System not registered before setting signature.");
        signatures[key] = signature;
    }

    void EntityDestroyed(Entity entity);
    void EntitySignatureChanged(Entity entity, EntitySignature entitySignature);

private:
    std::unordered_map<std::type_index, EntitySignature>        signatures{};
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};
};
