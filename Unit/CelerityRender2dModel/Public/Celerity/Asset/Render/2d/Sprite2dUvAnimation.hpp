#pragma once

#include <CelerityRender2dModelApi.hpp>

#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Item of animation frames array inside Sprite2dUvAnimationAsset.
struct CelerityRender2dModelApi Sprite2dUvAnimationFrameInfo final
{
    /// \brief UV coordinates for this frame.
    Math::AxisAlignedBox2d uv {{0.0f, 0.0f}, {1.0f, 1.0f}};

    /// \brief Duration of this frame in seconds.
    float durationS = 1.0f;

    struct CelerityRender2dModelApi Reflection final
    {
        StandardLayout::FieldId uv;
        StandardLayout::FieldId durationS;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Content of sprite uv animation file that stores all the information about this animation.
struct CelerityRender2dModelApi Sprite2dUvAnimationAsset final
{
    /// \brief Id of a material instance that is used along with this sprite animation.
    Memory::UniqueString materialInstanceId;

    /// \brief Array of this animation frames.
    Container::Vector<Sprite2dUvAnimationFrameInfo> frames {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"Sprite2dUvAnimationFrames"}}};

    struct CelerityRender2dModelApi Reflection final
    {
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::FieldId frames;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
