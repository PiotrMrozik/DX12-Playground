#pragma once
#include <cstdint>

struct MeshComponent
{
    std::uint32_t meshIndex{ 0 }; // Index into a mesh registry (no DX objects here)

    explicit MeshComponent(std::uint32_t index = 0) : meshIndex(index) {}
};
