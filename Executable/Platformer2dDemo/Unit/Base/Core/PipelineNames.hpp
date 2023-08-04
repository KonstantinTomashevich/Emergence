#pragma once

#include <Platformer2dDemoBaseApi.hpp>

#include <Memory/UniqueString.hpp>

struct Platformer2dDemoBaseApi PipelineNames
{
    PipelineNames () = delete;

    static const Emergence::Memory::UniqueString ROOT_NORMAL;
    static const Emergence::Memory::UniqueString GAME_ROOT_NORMAL;
    static const Emergence::Memory::UniqueString MAIN_MENU_LOADING_FIXED;
    static const Emergence::Memory::UniqueString MAIN_MENU_LOADING_NORMAL;
    static const Emergence::Memory::UniqueString MAIN_MENU_READY_FIXED;
    static const Emergence::Memory::UniqueString MAIN_MENU_READY_NORMAL;
    static const Emergence::Memory::UniqueString PLATFORMER_LOADING_FIXED;
    static const Emergence::Memory::UniqueString PLATFORMER_LOADING_NORMAL;
    static const Emergence::Memory::UniqueString PLATFORMER_GAME_FIXED;
    static const Emergence::Memory::UniqueString PLATFORMER_GAME_NORMAL;
};
