#pragma once

#include <Memory/UniqueString.hpp>

struct EffectConstant final
{
    EffectConstant () = delete;

    static const Emergence::Memory::UniqueString DEATH_TAG;
};
