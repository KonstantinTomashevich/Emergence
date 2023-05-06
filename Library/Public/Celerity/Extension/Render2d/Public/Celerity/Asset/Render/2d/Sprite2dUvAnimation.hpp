#pragma once

#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Content of sprite uv animation header file that stores main information about this animation.
/// \invariant Path to animation header file is <animation root folder>/<animation id>.animation.<format extension>,
///            where animation root folder is any registered root folder for animations, animation id is any string
///            that may include '/' for folder grouping, and format extension is either yaml or bin.
struct Sprite2dUvAnimationAssetHeader final
{
    /// \brief Id of a material instance that is used along with this sprite animation.
    Memory::UniqueString materialInstanceId;

    struct Reflection final
    {
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Item of frames bundle file that contains all frames of sprite 2d uv animation.
/// \invariant Path to frames bundle file is <animation root folder>/<animation id>.frames.<format extension>,
///            where animation root folder is any registered root folder for animations, animation id is any string
///            that may include '/' for folder grouping, and format extension is either yaml or bin.
struct Sprite2dUvAnimationFrameBundleItem final
{
    /// \brief UV coordinates for this frame.
    Math::AxisAlignedBox2d uv {{0.0f, 0.0f}, {1.0f, 1.0f}};

    /// \brief Duration of this frame in seconds.
    float durationS = 1.0f;

    struct Reflection final
    {
        StandardLayout::FieldId uv;
        StandardLayout::FieldId durationS;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
