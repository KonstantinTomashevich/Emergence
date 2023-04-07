#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Platformer/Movement/MovementState.hpp>

#include <StandardLayout/Mapping.hpp>

struct MovementComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString configurationId;

    MovementState state = MovementState::IDLE;

    uint64_t stateStartTimeNs = 0u;

    Emergence::Math::Vector2f lastMovementVelocity = Emergence::Math::Vector2f::ZERO;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId configurationId;
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::FieldId stateStartTimeNs;
        Emergence::StandardLayout::FieldId lastMovementVelocity;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
