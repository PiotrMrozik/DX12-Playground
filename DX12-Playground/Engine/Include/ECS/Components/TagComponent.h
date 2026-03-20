#pragma once
#include <string>

struct TagComponent
{
    std::string name;

    explicit TagComponent(std::string tag = "Entity") : name(std::move(tag)) {}
};
