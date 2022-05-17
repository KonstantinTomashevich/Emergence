#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Physics
{
struct PhysicsWorldSingleton final
{
    PhysicsWorldSingleton () noexcept = default;

    PhysicsWorldSingleton (const PhysicsWorldSingleton &_other) = delete;

    PhysicsWorldSingleton (PhysicsWorldSingleton &&_other) = delete;

    ~PhysicsWorldSingleton () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PhysicsWorldSingleton);

    Math::Vector3f gravity {0.0f, -9.81f, 0.0f};
    float toleranceLength = 1.0f;
    float toleranceSpeed = 10.0f;
    bool enableVisualDebugger = false;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId gravity;
        Emergence::StandardLayout::FieldId toleranceLength;
        Emergence::StandardLayout::FieldId toleranceSpeed;
        Emergence::StandardLayout::FieldId enableVisualDebugger;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
