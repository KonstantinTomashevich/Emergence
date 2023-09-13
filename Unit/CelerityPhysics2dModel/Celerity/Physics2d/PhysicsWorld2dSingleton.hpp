#pragma once


#include <CelerityPhysics2dModelApi.hpp>
#include <array>
#include <thread>

#include <API/Common/Shortcuts.hpp>

#include <Math/Vector2f.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters of physics simulation.
struct CelerityPhysics2dModelApi PhysicsWorld2dSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysicsWorld2dSingleton);

    /// \brief Whether physics simulation should be enabled and running.
    bool simulating = true;

    /// \brief Global world gravity.
    /// \details Can be changed from fixed pipeline.
    Math::Vector2f gravity {0.0f, -9.81f};

    /// \brief For each collision type contains mask of other collision types with which this type collides.
    /// \note To avoid excessive duplication, types A and B collide when
    ///       `(masks[A] & (1 << B)) | (masks[B] & (1 << A))`. Notice `|` operation in the center -- it means that
    ///       if A collides with B, you need to fill only one of the masks.
    /// \details Can be changed from fixed pipeline. By default filled with 1 bits for everything.
    std::array<std::uint32_t, 32u> collisionMasks;

    /// \brief Atomic counter for generating unique ids for collision shapes.
    /// \invariant Do not access directly, use ::GenerateShapeId.
    std::atomic_uintptr_t shapeIdCounter = 0u;

    /// \brief Atomic counter for generating unique ids for collision contacts.
    /// \invariant Do not access directly, use ::GenerateCollisionContactId.
    std::atomic_uintptr_t collisionContactIdCounter = 0u;

    /// \brief Atomic counter for generating unique ids for trigger contacts.
    /// \invariant Do not access directly, use ::GenerateTriggerContactId.
    std::atomic_uintptr_t triggerContactIdCounter = 0u;

    /// \brief Block with implementation-specific data.
    std::array<std::uint8_t, sizeof (std::uintptr_t)> implementationBlock;

    /// \brief Generates new unique id for a collision shape.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateShapeId () const noexcept;

    /// \brief Generates new unique id for a collision contact.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateCollisionContactId () const noexcept;

    /// \brief Generates new unique id for a trigger contact.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateTriggerContactId () const noexcept;

    struct CelerityPhysics2dModelApi Reflection final
    {
        StandardLayout::FieldId simulating;
        StandardLayout::FieldId gravity;
        std::array<StandardLayout::FieldId, 32u> collisionMasks;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
