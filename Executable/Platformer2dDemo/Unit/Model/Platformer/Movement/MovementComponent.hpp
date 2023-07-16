#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Platformer/Movement/MovementDirection.hpp>
#include <Platformer/Movement/MovementState.hpp>

#include <StandardLayout/Mapping.hpp>

struct MovementComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString configurationId;

    MovementState state = MovementState::IDLE;

    MovementDirection lastMovementDirection = MovementDirection::RIGHT;

    std::uint64_t stateStartTimeNs = 0u;

    Emergence::Math::Vector2f lastMovementVelocity = Emergence::Math::Vector2f::ZERO;

    /// \details Due to the collision issues, described bellow, we cannot just rely on whether we have ground collision
    ///          this frame, we need to wait one frame after this collision removal or addition.
    ///          - Despite the fact that tile colliders have no holes between them, floating point precision sometimes
    ///            detects loss of collision with one of the tiles, but isn't yet able to detect collision with the
    ///            other tile. Therefore, we're left without ground collision for one frame.
    ///          - Some collision removals are detected one frame later than they happened for some reason.
    ///            This problem usually occurs with jumps, when foot circle is already in the air, but ground
    ///            collision is still reported for some reason.
    std::uint64_t framesInAir = 0u;

    /// \details See ::framesInAir.
    std::uint64_t framesOnGround = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId configurationId;
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::FieldId lastMovementDirection;
        Emergence::StandardLayout::FieldId stateStartTimeNs;
        Emergence::StandardLayout::FieldId lastMovementVelocity;
        Emergence::StandardLayout::FieldId framesInAir;
        Emergence::StandardLayout::FieldId framesOnGround;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
