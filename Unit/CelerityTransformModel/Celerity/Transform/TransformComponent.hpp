#pragma once

#include <CelerityTransformModelApi.hpp>

#include <limits>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Transform2d.hpp>
#include <Math/Transform3d.hpp>

namespace Emergence::Celerity
{
template <typename Transform>
class TransformWorldAccessor;

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
template <typename Transform>
class CelerityTransformModelApi TransformComponent final
{
public:
    TransformComponent () noexcept = default;

    /// \return Id of an object to which component is attached.
    UniqueId GetObjectId () const noexcept;

    /// \brief Set id of an object to which component is attached.
    void SetObjectId (UniqueId _objectId) noexcept;

    /// \return Id of the parent object in transform hierarchy.
    UniqueId GetParentObjectId () const noexcept;

    /// \brief Set id of the parent object in transform hierarchy.
    void SetParentObjectId (UniqueId _parentObjectId) noexcept;

    /// \return Local logical (see class details) transform.
    const Transform &GetLogicalLocalTransform () const noexcept;

    /// \brief Set local logical (see class details) transform.
    /// \param _skipInterpolation If true, visual position will be set right away instead of being interpolated.
    void SetLogicalLocalTransform (const Transform &_transform, bool _skipInterpolation = false) noexcept;

    /// \return Global logical (see class details) transform.
    const Transform &GetLogicalWorldTransform (TransformWorldAccessor<Transform> &_accessor) const noexcept;

    /// \return Local visual (see class details) transform.
    const Transform &GetVisualLocalTransform () const noexcept;

    /// \brief Set local visual (see class details) transform.
    void SetVisualLocalTransform (const Transform &_transform) noexcept;

    /// \return Global visual (see class details) transform.
    const Transform &GetVisualWorldTransform (TransformWorldAccessor<Transform> &_accessor) const noexcept;

private:
    template <typename TransformComponentType>
    friend class TransformVisualSynchronizer;

    /// \brief Used to inform transform caching logic that parent transform was never observed yet.
    static constexpr std::size_t UNKNOWN_REVISION = std::numeric_limits<UniqueId>::max ();

    /// \return Whether cache was actually changed.
    bool UpdateLogicalWorldTransformCache (TransformWorldAccessor<Transform> &_accessor) const noexcept;

    /// \return Whether cache was actually changed.
    bool UpdateVisualWorldTransformCache (TransformWorldAccessor<Transform> &_accessor) const noexcept;

    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId parentObjectId = INVALID_UNIQUE_ID;

    Transform logicalLocalTransform {};
    std::uint64_t logicalLocalTransformRevision = 0u;
    mutable std::uint64_t logicalLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    mutable bool logicalLocalTransformChangedSinceLastUpdate = true;
    mutable Transform logicalWorldTransformCache {};

    Transform visualLocalTransform {};
    std::uint64_t visualLocalTransformRevision = 0u;
    mutable std::uint64_t visualLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    mutable bool visualLocalTransformChangedSinceLastUpdate = true;
    mutable Transform visualWorldTransformCache {};

    bool visualTransformSyncNeeded = false;

    std::uint64_t lastObservedLogicalTransformRevision = 0u;
    std::uint64_t logicalTransformLastObservationTimeNs = 0u;
    std::uint64_t visualTransformLastSyncTimeNs = 0u;

public:
    struct CelerityTransformModelApi Reflection final
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

using Transform2dComponent = TransformComponent<Math::Transform2d>;
using Transform3dComponent = TransformComponent<Math::Transform3d>;
} // namespace Emergence::Celerity
