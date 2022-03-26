#pragma once

#include <Celerity/Constants.hpp>

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

    Celerity::ObjectId GetObjectId () const noexcept;

    void SetObjectId (Celerity::ObjectId _objectId) noexcept;

    Celerity::ObjectId GetParentObjectId () const noexcept;

    void SetParentObjectId (Celerity::ObjectId _parentObjectId) noexcept;

    const Math::Transform3d &GetLogicalLocalTransform () const noexcept;

    void SetLogicalLocalTransform (const Math::Transform3d &_transform, bool _skipInterpolation = false) noexcept;

    const Math::Transform3d &GetLogicalWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

    const Math::Transform3d &GetVisualLocalTransform () const noexcept;

    void SetVisualLocalTransform (const Math::Transform3d &_transform) noexcept;

    const Math::Transform3d &GetVisualWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

private:
    friend class Transform3dVisualSynchronizer;

    /// \return Whether cache was actually changed.
    bool UpdateLogicalWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    /// \return Whether cache was actually changed.
    bool UpdateVisualWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept;

    Celerity::ObjectId objectId = Celerity::INVALID_OBJECT_ID;
    Celerity::ObjectId parentObjectId = Celerity::INVALID_OBJECT_ID;

    Math::Transform3d logicalLocalTransform;
    uint64_t logicalLocalTransformRevision = 0u;
    mutable uint64_t logicalParentTransformRevision = 0u;
    mutable Math::Transform3d logicalWorldTransform;

    Math::Transform3d visualLocalTransform;
    uint64_t visualLocalTransformRevision = 0u;
    mutable uint64_t visualParentTransformRevision = 0u;
    mutable Math::Transform3d visualWorldTransform;

    bool visualTransformSyncNeeded = false;

    /// \details We need to skip interpolation for transform initialization.
    bool interpolationSkipRequested = true;

    uint64_t lastObservedLogicalTransformRevision = 0u;
    uint64_t logicalTransformLastObservationTimeNs = 0u;
    uint64_t visualTransformLastSyncTimeNs = 0u;

public:
    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId parentObjectId;
        Emergence::StandardLayout::FieldId logicalLocalTransform;
        Emergence::StandardLayout::FieldId visualLocalTransform;
        Emergence::StandardLayout::FieldId visualTransformSyncNeeded;
        Emergence::StandardLayout::FieldId interpolationSkipRequested;
        Emergence::StandardLayout::FieldId lastObservedLogicalTransformRevision;
        Emergence::StandardLayout::FieldId logicalTransformLastObservationTimeNs;
        Emergence::StandardLayout::FieldId visualTransformLastSyncTimeNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Transform
