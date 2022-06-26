#pragma once

#include <array>
#include <thread>

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Celerity
{
struct PhysicsWorldSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysicsWorldSingleton);

    float toleranceLength = 1.0f;
    float toleranceSpeed = 10.0f;

    uint8_t simulationMaxThreads = static_cast<uint8_t> (std::thread::hardware_concurrency ());
    bool enableMemoryProfiling = false;

    /// \details Can be changed from fixed pipeline.
    Math::Vector3f gravity {0.0f, -9.81f, 0.0f};

    /// \details Can be changed from fixed pipeline.
    std::array<uint32_t, 32u> collisionMasks;

    /// \details Can be changed at any time, even from other pipelines than fixed.
    bool enableRemoteDebugger = false;

    std::array<char, 32u> remoteDebuggerUrl = {'\0'};
    uint32_t remoteDebuggerPort = 0u;

    /// \invariant Do not access directly, use ::GenerateShapeUID.
    std::atomic_unsigned_lock_free shapeUIDCounter = 0u;

    std::array<uint8_t, 88u> implementationBlock;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateShapeUID () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId toleranceLength;
        StandardLayout::FieldId toleranceSpeed;
        StandardLayout::FieldId simulationMaxThreads;
        StandardLayout::FieldId enableMemoryProfiling;
        StandardLayout::FieldId gravity;
        StandardLayout::FieldId collisionMasksBlock;
        std::array<StandardLayout::FieldId, 32u> collisionMasks;
        StandardLayout::FieldId enableRemoteDebugger;
        StandardLayout::FieldId remoteDebuggerUrl;
        StandardLayout::FieldId remoteDebuggerPort;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
