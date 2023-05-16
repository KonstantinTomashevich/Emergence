#pragma once

#include <Container/Vector.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes single frame of an Sprite2dUvAnimation.
struct Sprite2dUvAnimationFrame final
{
    /// \brief Value for Sprite2dComponent::uv while this frame is active.
    Math::AxisAlignedBox2d uv {{0.0f, 0.0f}, {1.0f, 1.0f}};

    /// \brief Indicates how long is this frame in nanoseconds.
    uint64_t durationNs = 0;

    /// \brief Moment of animation time after which this frame becomes active.
    uint64_t startTimeNs = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId uv;
        StandardLayout::FieldId durationNs;
        StandardLayout::FieldId startTimeNs;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Animates sprite using a set of uv coordinates as animation frames.
struct Sprite2dUvAnimation final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Material instances that is assigned to sprite to which animation is attached,
    Memory::UniqueString materialInstanceId;

    /// \details We store frames in vector instead of asset components,
    ///          because we need a fast access with strict order.
    Container::Vector<Sprite2dUvAnimationFrame> frames {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::FieldId frames;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
