#pragma once

#include <StandardLayout/Mapping.hpp>

/// \brief Internal singleton, used to register fetch and modify access to Urho3D engine objects.
struct Urho3DAccessSingleton final
{
    struct Reflection
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
