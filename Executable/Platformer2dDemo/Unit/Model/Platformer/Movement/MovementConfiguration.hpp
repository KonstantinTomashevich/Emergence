#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct MovementConfiguration final
{
    Emergence::Memory::UniqueString id;

    float runVelocity = 0.0f;

    float jumpVelocity = 0.0f;

    float groundMaxSlopeDeg = 0.0f;

    float airControlMaxVelocity = 0.0f;

    float airControlAcceleration = 0.0f;

    float rollVelocity = 0.0f;

    float rollDurationS = 0.0f;

    float slideVelocity = 0.0f;

    float slideDurationS = 0.0f;

    bool allowJump = false;

    bool allowCrouch = false;

    bool allowRoll = false;

    bool allowSlide = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId id;
        Emergence::StandardLayout::FieldId runVelocity;
        Emergence::StandardLayout::FieldId jumpVelocity;
        Emergence::StandardLayout::FieldId groundMaxSlopeDeg;
        Emergence::StandardLayout::FieldId airControlMaxVelocity;
        Emergence::StandardLayout::FieldId airControlAcceleration;
        Emergence::StandardLayout::FieldId rollVelocity;
        Emergence::StandardLayout::FieldId rollDurationS;
        Emergence::StandardLayout::FieldId slideVelocity;
        Emergence::StandardLayout::FieldId slideDurationS;
        Emergence::StandardLayout::FieldId allowJump;
        Emergence::StandardLayout::FieldId allowCrouch;
        Emergence::StandardLayout::FieldId allowRoll;
        Emergence::StandardLayout::FieldId allowSlide;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
