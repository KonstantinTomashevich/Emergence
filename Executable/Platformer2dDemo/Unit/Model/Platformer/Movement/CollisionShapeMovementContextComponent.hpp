#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Platformer/Movement/MovementState.hpp>

#include <StandardLayout/Mapping.hpp>

struct CollisionShapeMovementContextComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Celerity::UniqueId shapeId = Emergence::Celerity::INVALID_UNIQUE_ID;

    MovementStateFlag supportedStates = MovementStateFlag::NONE;

    bool useForGroundContactCheck = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId shapeId;
        Emergence::StandardLayout::FieldId supportedStates;
        Emergence::StandardLayout::FieldId useForGroundContactCheck;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
