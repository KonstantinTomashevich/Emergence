#pragma once

#include <Memory/UniqueString.hpp>

struct WorldStates final
{
    WorldStates () = delete;

    static const Emergence::Memory::UniqueString MAIN_MENU;
    static const Emergence::Memory::UniqueString GAME_LEVEL;
};
