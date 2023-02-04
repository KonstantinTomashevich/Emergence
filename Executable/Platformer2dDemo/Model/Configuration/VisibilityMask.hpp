#pragma once

#include <cstdint>

enum class VisibilityMask : uint64_t
{
    GAME_SCENE = uint64_t {1u} << uint64_t {0u},
    LOADING_ANIMATION = uint64_t {1u} << uint64_t {63u},
};
