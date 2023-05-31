#pragma once

#include <cstdint>

enum class VisibilityMask : std::uint64_t
{
    GAME_SCENE = std::uint64_t {1u} << std::uint64_t {0u},
    LOADING_ANIMATION = std::uint64_t {1u} << std::uint64_t {1u},
};
