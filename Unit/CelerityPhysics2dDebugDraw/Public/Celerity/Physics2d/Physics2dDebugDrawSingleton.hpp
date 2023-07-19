#pragma once

#include <CelerityPhysics2dDebugDrawApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains configuration for 2d physics debug draw feature.
struct CelerityPhysics2dDebugDrawApi Physics2dDebugDrawSingleton final
{
    /// \brief Whether debug draw feature is enabled right now.
    bool enabled = false;

    /// \brief Internal field for correctly updating ::enabled changes.
    /// \details We could've used on change event for this, but event
    ///          for tracking one bool field looks like an overkill.
    bool wasEnabled = false;

    struct CelerityPhysics2dDebugDrawApi Reflection final
    {
        StandardLayout::FieldId enabled;
        StandardLayout::FieldId wasEnabled;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
