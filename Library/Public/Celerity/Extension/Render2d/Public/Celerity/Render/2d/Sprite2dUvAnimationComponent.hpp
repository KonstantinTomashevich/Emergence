#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Attaches uv-based animation to Sprite2dComponent.
struct Sprite2dUvAnimationComponent final
{
    /// \brief Id of an object with Transform2dComponent to which this sprite is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Id of a sprite to which this animation is attached.
    UniqueId spriteId = INVALID_UNIQUE_ID;

    /// \brief Id of Sprite2dUvAnimation asset that describes the animation.
    Memory::UniqueString animationId;

    /// \brief Animation local time, used to select appropriate ::currentFrame.
    uint64_t currentTimeNs = 0u;

    /// \brief Index of animation frame that is applied to sprite right now.
    uint64_t currentFrame = std::numeric_limits<uint32_t>::max ();

    /// \brief Last moment of time during which animation sync was done.
    /// \details Needed for internal logic in order to avoid ticking several
    ///          times on animations that are visible in several viewports.
    uint64_t lastSyncNormalTimeNs = 0u;

    /// \brief Whether animation ::currentTimeNs should be automatically incremented.
    /// \details ::currentTimeNs is automatically incremented only on visible animations!
    bool tickTime = true;

    /// \brief Whether animation is looped.
    bool loop = false;

    /// \brief Whether animation is finished.
    /// \warning If ::loop, animation cannot be finished at all!
    bool finished = false;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId spriteId;
        StandardLayout::FieldId animationId;
        StandardLayout::FieldId currentTimeNs;
        StandardLayout::FieldId currentFrame;
        StandardLayout::FieldId lastSyncNormalTimeNs;
        StandardLayout::FieldId tickTime;
        StandardLayout::FieldId loop;
        StandardLayout::FieldId finished;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
