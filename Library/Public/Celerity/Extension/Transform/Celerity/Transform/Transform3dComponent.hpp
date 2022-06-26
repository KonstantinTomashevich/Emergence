#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Transform3d.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence::Celerity
{
class Transform3dWorldAccessor;

/// \brief Represents transform hierarchy node attached to an object.
/// \details There are two types of object transform:
///          - Logical, used for deterministic simulation in fixed update pipeline.
///          - Visual, used for object rendering in normal update pipeline.
///          We can not use logical transform for rendering, because it will cause tearing.
///
///          If object is used inside gameplay logic, only its logical transform should be changed:
///          task from Transform::VisualSync::AddToNormalUpdate will take care of synchronizing and smoothing.
///
///          If object is used only for visual effects, only its visual transform should be changed,
///          because it has no "logical" gameplay meaning.
class Transform3dComponent final
{
public:
    Transform3dComponent () noexcept = default;

    /// \return Id of an object to which component is attached.
    UniqueId GetObjectId () const noexcept;

    /// \brief Set id of an object to which component is attached.
    void SetObjectId (UniqueId _objectId) noexcept;

    /// \return Id of the parent object in transform hierarchy.
    UniqueId GetParentObjectId () const noexcept;

    /// \brief Set id of the parent object in transform hierarchy.
    void SetParentObjectId (UniqueId _parentObjectId) noexcept;

    /// \return Local logical (see class details) transform.
    const Math::Transform3d &GetLogicalLocalTransform () const noexcept;

    /// \brief Set local logical (see class details) transform.
    /// \param _skipInterpolation If true, visual position will be set right away instead of being interpolated.
    void SetLogicalLocalTransform (const Math::Transform3d &_transform, bool _skipInterpolation = false) noexcept;

    /// \return Global logical (see class details) transform.
    const Math::Transform3d &GetLogicalWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

    /// \return Local visual (see class details) transform.
    const Math::Transform3d &GetVisualLocalTransform () const noexcept;

    /// \brief Set local visual (see class details) transform.
    void SetVisualLocalTransform (const Math::Transform3d &_transform) noexcept;

    /// \return Global visual (see class details) transform.
    const Math::Transform3d &GetVisualWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

private:
    friend class Transform3dVisualSynchronizer;

    /// \brief Used to inform transform caching logic that parent transform was never observed yet.
    static constexpr std::size_t UNKNOWN_REVISION = std::numeric_limits<UniqueId>::max ();

    /// \return Whether cache was actually changed.
    bool UpdateLogicalWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    /// \return Whether cache was actually changed.
    bool UpdateVisualWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId parentObjectId = INVALID_UNIQUE_ID;

    Math::Transform3d logicalLocalTransform;
    uint64_t logicalLocalTransformRevision = 0u;
    mutable uint64_t logicalLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    mutable bool logicalLocalTransformChangedSinceLastUpdate = false;
    mutable Math::Transform3d logicalWorldTransformCache;

    Math::Transform3d visualLocalTransform;
    uint64_t visualLocalTransformRevision = 0u;
    mutable uint64_t visualLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    mutable bool visualLocalTransformChangedSinceLastUpdate = false;
    mutable Math::Transform3d visualWorldTransformCache;

    bool visualTransformSyncNeeded = false;

    uint64_t lastObservedLogicalTransformRevision = 0u;
    uint64_t logicalTransformLastObservationTimeNs = 0u;
    uint64_t visualTransformLastSyncTimeNs = 0u;

public:
    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId parentObjectId;
        StandardLayout::FieldId logicalLocalTransform;
        StandardLayout::FieldId visualLocalTransform;
        StandardLayout::FieldId visualTransformSyncNeeded;
        StandardLayout::FieldId lastObservedLogicalTransformRevision;
        StandardLayout::FieldId logicalTransformLastObservationTimeNs;
        StandardLayout::FieldId visualTransformLastSyncTimeNs;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
