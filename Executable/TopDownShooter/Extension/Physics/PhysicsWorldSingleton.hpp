#pragma once

#include <array>
#include <thread>

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Physics
{
struct PhysicsWorldSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysicsWorldSingleton);

    Math::Vector3f gravity {0.0f, -9.81f, 0.0f};

    float toleranceLength = 1.0f;
    float toleranceSpeed = 10.0f;

    uint8_t simulationMaxThreads = std::thread::hardware_concurrency ();
    bool enableMemoryProfiling = false;

    bool enableRemoteDebugger = false;
    std::array<char, 32u> remoteDebuggerUrl = {'\0'};
    uint32_t remoteDebuggerPort = 0u;

    /// \invariant Do not access directly, use ::GenerateShapeUID.
    std::atomic_unsigned_lock_free shapeUIDCounter = 0u;

    /// \invariant Can not be changed after first physics simulation during fixed frame.
    ///            It is advised to fill this data from custom configuration routine.
    std::array<uint32_t, 32u> collisionMasks;

    std::array<uint8_t, 88u> implementationBlock;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateShapeUID () const noexcept;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId gravity;
        Emergence::StandardLayout::FieldId toleranceLength;
        Emergence::StandardLayout::FieldId toleranceSpeed;
        Emergence::StandardLayout::FieldId enableRemoteDebugger;
        Emergence::StandardLayout::FieldId remoteDebuggerUrl;
        Emergence::StandardLayout::FieldId remoteDebuggerPort;
        std::array<Emergence::StandardLayout::FieldId, 32u> collisionMasks;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
