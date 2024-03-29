#pragma once

#include <CelerityPhysics3dModelApi.hpp>

#include <array>
#include <thread>

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters of physics simulation.
struct CelerityPhysics3dModelApi PhysicsWorld3dSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysicsWorld3dSingleton);

    /// \brief Average object size. Needed to adjust floating error tolerance.
    /// \invariant Can not be changed after first fixed update.
    float toleranceLength = 1.0f;

    /// \brief Average object speed. Needed to adjust floating error tolerance.
    /// \invariant Can not be changed after first fixed update.
    float toleranceSpeed = 10.0f;

    /// \brief Maximum count of threads, that can be used to run physics simulation
    ///        (some implementations can not use pipeline threads).
    /// \invariant Can not be changed after first fixed update.
    std::uint8_t simulationMaxThreads = static_cast<std::uint8_t> (std::thread::hardware_concurrency ());

    /// \brief Whether Emergence MemoryProfiler should be enabled for physics allocations.
    /// \details Enabling MemoryProfiler can increase memory usage or decrease performance.
    /// \invariant Can not be changed after first fixed update.
    bool enableMemoryProfiling = false;

    /// \brief Whether physics simulation should be enabled and running.
    bool simulating = true;

    /// \brief Global world gravity.
    /// \details Can be changed from fixed pipeline.
    Math::Vector3f gravity {0.0f, -9.81f, 0.0f};

    /// \brief For each collision type contains mask of other collision types with which this type collides.
    /// \note To avoid excessive duplication, types A and B collide when
    ///       `(masks[A] & (1 << B)) | (masks[B] & (1 << A))`. Notice `|` operation in the center -- it means that
    ///       if A collides with B, you need to fill only one of the masks.
    /// \details Can be changed from fixed pipeline.
    std::array<std::uint32_t, 32u> collisionMasks;

    /// \brief Whether physics world should try to connect to remote debugger.
    /// \details Can be changed at any time, even from other pipelines than fixed.
    bool enableRemoteDebugger = false;

    /// \brief Remote debugger url.
    /// \warning Changes are applied only if reconnection happens.
    std::array<char, 32u> remoteDebuggerUrl = {'\0'};

    /// \brief Remote debugger port.
    /// \warning Changes are applied only if reconnection happens.
    std::uint32_t remoteDebuggerPort = 0u;

    /// \brief Atomic counter for generating unique ids for collision shapes.
    /// \invariant Do not access directly, use ::GenerateShapeId.
    std::atomic_uintptr_t shapeIdCounter = 0u;

    /// \brief Block with implementation-specific data.
    std::array<std::uint8_t, 88u> implementationBlock;

    /// \brief Generates new unique id for a collision shape.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    std::uintptr_t GenerateShapeId () const noexcept;

    struct CelerityPhysics3dModelApi Reflection final
    {
        StandardLayout::FieldId toleranceLength;
        StandardLayout::FieldId toleranceSpeed;
        StandardLayout::FieldId simulationMaxThreads;
        StandardLayout::FieldId enableMemoryProfiling;
        StandardLayout::FieldId simulating;
        StandardLayout::FieldId gravity;
        std::array<StandardLayout::FieldId, 32u> collisionMasks;
        StandardLayout::FieldId enableRemoteDebugger;
        StandardLayout::FieldId remoteDebuggerUrl;
        StandardLayout::FieldId remoteDebuggerPort;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
