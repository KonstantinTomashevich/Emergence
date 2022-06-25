#pragma once

#include <Memory/UniqueString.hpp>

struct InputConstant final
{
    InputConstant () = delete;

    static const Emergence::Memory::UniqueString MOVEMENT_ACTION_GROUP;
    static const Emergence::Memory::UniqueString FIGHT_ACTION_GROUP;

    static const Emergence::Memory::UniqueString MOTION_FACTOR_ACTION;
    static const Emergence::Memory::UniqueString ROTATION_FACTOR_ACTION;
    static const Emergence::Memory::UniqueString FIRE_ACTION;
    static const Emergence::Memory::UniqueString SLOWDOWN_ACTION;
};
