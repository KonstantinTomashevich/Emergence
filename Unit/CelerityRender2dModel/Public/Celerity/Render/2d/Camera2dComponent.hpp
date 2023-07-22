#pragma once

#include <CelerityRender2dModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents orthographic camera in 2d world, attached to a game object.
struct CelerityRender2dModelApi Camera2dComponent final
{
    /// \brief Id of an object with Transform2dComponent to which this camera is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Half height of the orthographic projection.
    /// \details Width is adjusted automatically based on window aspect ratio.
    ///          Half height instead of the full height is expected in order
    ///          to avoid unnecessary multiplication during rendering.
    float halfOrthographicSize = 0.0f;

    /// \brief Visibility mask for filtering out unwanted drawables.
    /// \details Drawable will be rendered through this camera only if this
    ///          mask and drawable mask have at least one common set bit.
    ///          For example, it can be used to hide world drawables during loading.
    std::uint64_t visibilityMask = ~0u;

    struct CelerityRender2dModelApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId halfOrthographicSize;
        StandardLayout::FieldId visibilityMask;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
