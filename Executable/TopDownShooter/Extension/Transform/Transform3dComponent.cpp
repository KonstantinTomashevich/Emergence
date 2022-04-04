#include <StandardLayout/MappingRegistration.hpp>

#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Transform
{
Celerity::ObjectId Transform3dComponent::GetObjectId () const noexcept
{
    return objectId;
}

void Transform3dComponent::SetObjectId (Celerity::ObjectId _objectId) noexcept
{
    objectId = _objectId;
    logicalLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    visualLastUpdateParentTransformRevision = UNKNOWN_REVISION;
}

Celerity::ObjectId Transform3dComponent::GetParentObjectId () const noexcept
{
    return parentObjectId;
}

void Transform3dComponent::SetParentObjectId (Celerity::ObjectId _parentObjectId) noexcept
{
    parentObjectId = _parentObjectId;
}

const Math::Transform3d &Transform3dComponent::GetLogicalLocalTransform () const noexcept
{
    return logicalLocalTransform;
}

void Transform3dComponent::SetLogicalLocalTransform (const Math::Transform3d &_transform,
                                                     bool _skipInterpolation) noexcept
{
    logicalLocalTransform = _transform;
    ++logicalLocalTransformRevision;
    visualTransformSyncNeeded = true;
    interpolationSkipRequested |= _skipInterpolation;
}

const Math::Transform3d &Transform3dComponent::GetLogicalWorldTransform (
    Transform3dWorldAccessor &_accessor) const noexcept
{
    static std::atomic_flag lock;
    AtomicFlagGuard guard {lock};

    UpdateLogicalWorldTransformCache (_accessor);
    return logicalWorldTransform;
}

const Math::Transform3d &Transform3dComponent::GetVisualLocalTransform () const noexcept
{
    return visualLocalTransform;
}

void Transform3dComponent::SetVisualLocalTransform (const Math::Transform3d &_transform) noexcept
{
    visualLocalTransform = _transform;
    ++visualLocalTransformRevision;
}

const Math::Transform3d &Transform3dComponent::GetVisualWorldTransform (
    Transform3dWorldAccessor &_accessor) const noexcept
{
    static std::atomic_flag lock;
    AtomicFlagGuard guard {lock};

    UpdateVisualWorldTransformCache (_accessor);
    return visualWorldTransform;
}

#define CACHE_UPDATE_METHOD(MethodTag, VariableTag)                                                                    \
    auto cursor = _accessor.fetchTransform3dByObjectId.Execute (&parentObjectId);                                      \
    const auto *parent = static_cast<const Transform3dComponent *> (*cursor);                                          \
                                                                                                                       \
    if (parent)                                                                                                        \
    {                                                                                                                  \
        if (parent->Update##MethodTag##WorldTransformCache (_accessor) ||                                              \
            parent->VariableTag##LocalTransformRevision != VariableTag##LastUpdateParentTransformRevision ||           \
            VariableTag##LocalTransformRevision != VariableTag##LastUpdateLocalTransformRevision)                      \
        {                                                                                                              \
            /* TODO: We already had matrix representation of `parent->*WorldTransform` during                          \
                     `parent->Update*WorldTransformCache` call unless parent is a root node. Therefore,                \
                     we might try to optimize this method by introducing matrix caching. However, it would             \
                     slow down root node getters, because they would calculate unneeded matrix. */                     \
            VariableTag##WorldTransform = parent->VariableTag##WorldTransform * VariableTag##LocalTransform;           \
            VariableTag##LastUpdateParentTransformRevision = parent->VariableTag##LastUpdateLocalTransformRevision;    \
            VariableTag##LastUpdateLocalTransformRevision = VariableTag##LocalTransformRevision;                       \
            return true;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        return false;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    /* This component is a root node. */                                                                               \
    if (VariableTag##LocalTransformRevision != VariableTag##LastUpdateLocalTransformRevision)                          \
    {                                                                                                                  \
        VariableTag##WorldTransform = VariableTag##LocalTransform;                                                     \
        VariableTag##LastUpdateLocalTransformRevision = VariableTag##LocalTransformRevision;                           \
        return true;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    return false

bool Transform3dComponent::UpdateLogicalWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept
{
    CACHE_UPDATE_METHOD (Logical, logical);
}

bool Transform3dComponent::UpdateVisualWorldTransformCache (Transform3dWorldAccessor &_accessor) const noexcept
{
    CACHE_UPDATE_METHOD (Visual, visual);
}

const Transform3dComponent::Reflection &Transform3dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Transform3dComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_REGULAR (parentObjectId)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (logicalLocalTransform)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (visualLocalTransform)
        EMERGENCE_MAPPING_REGISTER_REGULAR (visualTransformSyncNeeded)
        EMERGENCE_MAPPING_REGISTER_REGULAR (interpolationSkipRequested)
        EMERGENCE_MAPPING_REGISTER_REGULAR (lastObservedLogicalTransformRevision)
        EMERGENCE_MAPPING_REGISTER_REGULAR (logicalTransformLastObservationTimeNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (visualTransformLastSyncTimeNs)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Transform
