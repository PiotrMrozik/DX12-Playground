#include <ECS/System/SystemManager.h>

void SystemManager::EntityDestroyed(Entity entity)
{
    for (auto& [key, system] : systems)
        system->entities.erase(entity);
}

void SystemManager::EntitySignatureChanged(Entity entity, EntitySignature entitySignature)
{
    for (auto& [key, system] : systems)
    {
        const auto& systemSignature = signatures[key];
        if ((entitySignature & systemSignature) == systemSignature)
            system->entities.insert(entity);
        else
            system->entities.erase(entity);
    }
}
