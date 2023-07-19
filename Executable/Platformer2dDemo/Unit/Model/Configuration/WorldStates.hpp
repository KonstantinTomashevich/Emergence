#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Memory/UniqueString.hpp>

struct Platformer2dDemoModelApi WorldStates final
{
    WorldStates () = delete;

    static const Emergence::Memory::UniqueString MAIN_MENU;
    static const Emergence::Memory::UniqueString PLATFORMER;
};
