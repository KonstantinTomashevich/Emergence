#pragma once

#include <Memory/UniqueString.hpp>

struct PhysicsConstant final
{
    PhysicsConstant () = delete;

    static const Emergence::Memory::UniqueString DEFAULT_MATERIAL_ID;

    static constexpr std::uint32_t GROUND_COLLISION_GROUP = 0u;
    static constexpr std::uint32_t OBSTACLE_COLLISION_GROUP = 1u;
    static constexpr std::uint32_t WARRIOR_COLLISION_GROUP = 2u;
    static constexpr std::uint32_t BULLET_COLLISION_GROUP = 3u;
    static constexpr std::uint32_t HIT_BOX_COLLISION_GROUP = 4u;
};
