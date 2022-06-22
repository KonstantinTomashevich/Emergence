#pragma once

#include <Memory/UniqueString.hpp>

class HardcodedPrototypes final
{
public:
    HardcodedPrototypes () = delete;

    static const Emergence::Memory::UniqueString FIGHTER;
    static const Emergence::Memory::UniqueString OBSTACLE;
    static const Emergence::Memory::UniqueString FLOOR_TILE;
    static const Emergence::Memory::UniqueString BULLET;
};
