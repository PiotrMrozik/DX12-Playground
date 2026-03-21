#pragma once
#include <bitset>
#include <cstdint>
#include <limits>

using Entity        = std::uint32_t;
using ComponentType = std::uint8_t;

constexpr Entity        INVALID_ENTITY  = (std::numeric_limits<Entity>::max)();
constexpr std::uint32_t MAX_ENTITIES    = 1000;
constexpr ComponentType MAX_COMPONENTS  = 32;

using EntitySignature = std::bitset<MAX_COMPONENTS>;
