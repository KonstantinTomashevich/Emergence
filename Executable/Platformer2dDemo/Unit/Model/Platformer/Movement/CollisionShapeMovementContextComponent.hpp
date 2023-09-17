#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Platformer/Movement/MovementState.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi CollisionShapeMovementContextComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Celerity::UniqueId shapeId = Emergence::Celerity::INVALID_UNIQUE_ID;

    MovementStateFlag supportedStates = MovementStateFlag::NONE;

    bool useForGroundContactCheck = false;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId shapeId;
        Emergence::StandardLayout::FieldId supportedStates;
        Emergence::StandardLayout::FieldId useForGroundContactCheck;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
