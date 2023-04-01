#pragma once

#include <array>
#include <thread>

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector2f.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters of physics simulation.
struct PhysicsWorld2dSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysicsWorld2dSingleton);

    /// \brief Whether Emergence MemoryProfiler should be enabled for physics allocations.
    /// \details Enabling MemoryProfiler can increase memory usage or decrease performance.
    /// \invariant Can not be changed after first fixed update.
    bool enableMemoryProfiling = false;

    /// \brief Global world gravity.
    /// \details Can be changed from fixed pipeline.
    Math::Vector2f gravity {0.0f, -9.81f};

    /// \brief For each collision type contains mask of other collision types with which this type collides.
    /// \note To avoid excessive duplication, types A and B collide when
    ///       `(masks[A] & (1 << B)) | (masks[B] & (1 << A))`. Notice `|` operation in the center -- it means that
    ///       if A collides with B, you need to fill only one of the masks.
    /// \details Can be changed from fixed pipeline.
    std::array<uint32_t, 32u> collisionMasks;

    /// \brief Atomic counter for generating unique ids for collision shapes.
    /// \invariant Do not access directly, use ::GenerateShapeId.
    std::atomic_unsigned_lock_free shapeIdCounter = 0u;

    /// \brief Atomic counter for generating unique ids for collision contacts.
    /// \invariant Do not access directly, use ::GenerateCollisionContactId.
    std::atomic_unsigned_lock_free collisionContactIdCounter = 0u;

    /// \brief Atomic counter for generating unique ids for trigger contacts.
    /// \invariant Do not access directly, use ::GenerateTriggerContactId.
    std::atomic_unsigned_lock_free triggerContactIdCounter = 0u;

    /// \brief Block with implementation-specific data.
    std::array<uint8_t, sizeof (uintptr_t)> implementationBlock;

    /// \brief Generates new unique id for a collision shape.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] uintptr_t GenerateShapeId () const noexcept;

    /// \brief Generates new unique id for a collision contact.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] uintptr_t GenerateCollisionContactId () const noexcept;

    /// \brief Generates new unique id for a trigger contact.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] uintptr_t GenerateTriggerContactId () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId enableMemoryProfiling;
        StandardLayout::FieldId gravity;
        std::array<StandardLayout::FieldId, 32u> collisionMasks;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
