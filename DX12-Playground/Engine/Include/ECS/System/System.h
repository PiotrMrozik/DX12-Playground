#pragma once
#include <unordered_set>

#include <ECS/Globals.h>

class System
{
public:
    virtual ~System() = default;

    std::unordered_set<Entity> entities{};
};
