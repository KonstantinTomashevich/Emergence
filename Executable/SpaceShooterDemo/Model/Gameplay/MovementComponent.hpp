#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

struct MovementComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    float maxLinearSpeed = 0.0f;
    Emergence::Math::Vector3f linearAcceleration = Emergence::Math::Vector3f::ZERO;

    float maxAngularSpeed = 0.0f;
    Emergence::Math::Vector3f angularAcceleration = Emergence::Math::Vector3f::ZERO;

    uint8_t linearVelocityMask = 0u;
    uint8_t angularVelocityMask = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId maxLinearSpeed;
        Emergence::StandardLayout::FieldId linearAcceleration;
        Emergence::StandardLayout::FieldId maxAngularSpeed;
        Emergence::StandardLayout::FieldId angularAcceleration;
        Emergence::StandardLayout::FieldId linearVelocityMask;
        Emergence::StandardLayout::FieldId angularVelocityMask;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
