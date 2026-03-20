#include <ECS/Component/ComponentManager.h>

void ComponentManager::EntityDestroyed(Entity entity)
{
    for (auto& [key, array] : componentArrays)
        array->EntityDestroyed(entity);
}
