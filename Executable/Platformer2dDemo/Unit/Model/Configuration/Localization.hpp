#pragma once

#include <Memory/UniqueString.hpp>

struct Localization final
{
    Localization () = delete;

    static const Emergence::Memory::UniqueString HARDCODED_LOCALE;
};
