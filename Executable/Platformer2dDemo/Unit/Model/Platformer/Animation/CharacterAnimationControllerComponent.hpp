#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

#include <Platformer/Movement/MovementDirection.hpp>

#include <StandardLayout/Mapping.hpp>

enum class CharacterAnimationState : std::uint8_t
{
    NONE = 0u,
    CROUCH,
    FALL,
    IDLE,
    JUMP,
    ROLL,
    RUN,
    SLIDE,
};

struct Platformer2dDemoModelApi CharacterAnimationControllerComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Celerity::UniqueId spriteId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString animationConfigurationId;

    CharacterAnimationState currentState = CharacterAnimationState::NONE;

    MovementDirection direction = MovementDirection::RIGHT;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId spriteId;
        Emergence::StandardLayout::FieldId animationConfigurationId;
        Emergence::StandardLayout::FieldId currentState;
        Emergence::StandardLayout::FieldId direction;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
