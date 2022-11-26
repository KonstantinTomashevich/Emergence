#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Internal singleton, used to register fetch and modify access to Box2d engine objects.
struct Box2dAccessSingleton final
{
    struct Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
