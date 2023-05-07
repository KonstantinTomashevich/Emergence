#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct CharacterAnimationConfiguration final
{
    Emergence::Memory::UniqueString id;
    Emergence::Memory::UniqueString crouchAnimationId;
    Emergence::Memory::UniqueString fallAnimationId;
    Emergence::Memory::UniqueString idleAnimationId;
    Emergence::Memory::UniqueString jumpAnimationId;
    Emergence::Memory::UniqueString rollAnimationId;
    Emergence::Memory::UniqueString runAnimationId;
    Emergence::Memory::UniqueString slideAnimationId;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId id;
        Emergence::StandardLayout::FieldId crouchAnimationId;
        Emergence::StandardLayout::FieldId fallAnimationId;
        Emergence::StandardLayout::FieldId idleAnimationId;
        Emergence::StandardLayout::FieldId jumpAnimationId;
        Emergence::StandardLayout::FieldId rollAnimationId;
        Emergence::StandardLayout::FieldId runAnimationId;
        Emergence::StandardLayout::FieldId slideAnimationId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
