#pragma once

#include <CelerityPhysics2dModelBox2dApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Internal singleton, used to register fetch and modify access to Box2d engine objects.
struct CelerityPhysics2dModelBox2dApi Box2dAccessSingleton final
{
    struct CelerityPhysics2dModelBox2dApi Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
