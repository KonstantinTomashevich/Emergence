#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Transform3d.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence::Transform
{
class Transform3dWorldAccessor;

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

    Celerity::UniqueId GetObjectId () const noexcept;

    void SetObjectId (Celerity::UniqueId _objectId) noexcept;

    Celerity::UniqueId GetParentObjectId () const noexcept;

    void SetParentObjectId (Celerity::UniqueId _parentObjectId) noexcept;

    const Math::Transform3d &GetLogicalLocalTransform () const noexcept;

    void SetLogicalLocalTransform (const Math::Transform3d &_transform, bool _skipInterpolation = false) noexcept;

    const Math::Transform3d &GetLogicalWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

    const Math::Transform3d &GetVisualLocalTransform () const noexcept;

    void SetVisualLocalTransform (const Math::Transform3d &_transform) noexcept;

    const Math::Transform3d &GetVisualWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

private:
    friend class Transform3dVisualSynchronizer;

    /// \brief Used to inform transform caching logic that parent transform was never observed yet.
    static constexpr std::size_t UNKNOWN_REVISION = std::numeric_limits<Celerity::UniqueId>::max ();

    /// \return Whether cache was actually changed.
    bool UpdateLogicalWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    /// \return Whether cache was actually changed.
    bool UpdateVisualWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Celerity::UniqueId parentObjectId = Celerity::INVALID_UNIQUE_ID;

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

    /// \details We need to skip interpolation for transform initialization.
    bool interpolationSkipRequested = true;

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
        StandardLayout::FieldId interpolationSkipRequested;
        StandardLayout::FieldId lastObservedLogicalTransformRevision;
        StandardLayout::FieldId logicalTransformLastObservationTimeNs;
        StandardLayout::FieldId visualTransformLastSyncTimeNs;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Transform
