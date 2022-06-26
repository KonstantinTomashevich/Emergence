#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Internal singleton, used to register fetch and modify access to PhysX engine objects.
struct PhysXAccessSingleton final
{
    struct Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
