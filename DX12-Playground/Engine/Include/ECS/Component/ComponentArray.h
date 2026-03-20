#pragma once
#include <array>
#include <cassert>
#include <unordered_map>

#include <ECS/Component/IComponentArray.h>

template <typename T>
class ComponentArray : public IComponentArray
{
public:
    void InsertData(Entity entity, T component)
    {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

        const size_t newIndex       = size;
        entityToIndexMap[entity]    = newIndex;
        indexToEntityMap[newIndex]  = entity;
        componentArray[newIndex]    = std::move(component);
        ++size;
    }

    void RemoveData(Entity entity)
    {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing non-existent component.");

        const size_t indexOfRemoved = entityToIndexMap[entity];
        const size_t indexOfLast    = size - 1;

        // Swap last element into the removed slot to keep array packed
        componentArray[indexOfRemoved] = std::move(componentArray[indexOfLast]);

        const Entity entityOfLast          = indexToEntityMap[indexOfLast];
        entityToIndexMap[entityOfLast]     = indexOfRemoved;
        indexToEntityMap[indexOfRemoved]   = entityOfLast;

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(indexOfLast);

        --size;
    }

    T& GetData(Entity entity)
    {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");
        return componentArray[entityToIndexMap[entity]];
    }

    void EntityDestroyed(Entity entity) override
    {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end())
            RemoveData(entity);
    }

private:
    std::array<T, MAX_ENTITIES> componentArray{};
    std::unordered_map<Entity, size_t> entityToIndexMap{};
    std::unordered_map<size_t, Entity> indexToEntityMap{};
    size_t size{};
};
