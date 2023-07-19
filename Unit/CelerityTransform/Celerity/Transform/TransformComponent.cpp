#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence::Celerity
{
template <typename Transform>
UniqueId TransformComponent<Transform>::GetObjectId () const noexcept
{
    return objectId;
}

template <typename Transform>
void TransformComponent<Transform>::SetObjectId (UniqueId _objectId) noexcept
{
    objectId = _objectId;
}

template <typename Transform>
UniqueId TransformComponent<Transform>::GetParentObjectId () const noexcept
{
    return parentObjectId;
}

template <typename Transform>
void TransformComponent<Transform>::SetParentObjectId (UniqueId _parentObjectId) noexcept
{
    parentObjectId = _parentObjectId;
    logicalLastUpdateParentTransformRevision = UNKNOWN_REVISION;
    visualLastUpdateParentTransformRevision = UNKNOWN_REVISION;
}

template <typename Transform>
const Transform &TransformComponent<Transform>::GetLogicalLocalTransform () const noexcept
{
    return logicalLocalTransform;
}

template <typename Transform>
void TransformComponent<Transform>::SetLogicalLocalTransform (const Transform &_transform,
                                                              bool _skipInterpolation) noexcept
{
    logicalLocalTransform = _transform;
    ++logicalLocalTransformRevision;
    logicalLocalTransformChangedSinceLastUpdate = true;

    if (_skipInterpolation)
    {
        SetVisualLocalTransform (logicalLocalTransform);
        visualTransformSyncNeeded = false;
    }
    else
    {
        if (!visualTransformSyncNeeded)
        {
            // We're starting new interpolated movement, therefore we need to clear time stamp of the old one.
            visualTransformLastSyncTimeNs = 0u;

            visualTransformSyncNeeded = true;
        }
    }
}

template <typename Transform>
const Transform &TransformComponent<Transform>::GetLogicalWorldTransform (
    TransformWorldAccessor<Transform> &_accessor) const noexcept
{
    static std::atomic_flag lock;
    AtomicFlagGuard guard {lock};

    UpdateLogicalWorldTransformCache (_accessor);
    return logicalWorldTransformCache;
}

template <typename Transform>
const Transform &TransformComponent<Transform>::GetVisualLocalTransform () const noexcept
{
    return visualLocalTransform;
}

template <typename Transform>
void TransformComponent<Transform>::SetVisualLocalTransform (const Transform &_transform) noexcept
{
    visualLocalTransform = _transform;
    ++visualLocalTransformRevision;
    visualLocalTransformChangedSinceLastUpdate = true;
}

template <typename Transform>
const Transform &TransformComponent<Transform>::GetVisualWorldTransform (
    TransformWorldAccessor<Transform> &_accessor) const noexcept
{
    static std::atomic_flag lock;
    AtomicFlagGuard guard {lock};

    UpdateVisualWorldTransformCache (_accessor);
    return visualWorldTransformCache;
}

#define CACHE_UPDATE_METHOD(MethodTag, VariableTag)                                                                    \
    auto cursor = _accessor.fetchTransformByObjectId.Execute (&parentObjectId);                                        \
    const auto *parent = static_cast<const TransformComponent *> (*cursor);                                            \
                                                                                                                       \
    if (parent)                                                                                                        \
    {                                                                                                                  \
        if (parent->Update##MethodTag##WorldTransformCache (_accessor) ||                                              \
            parent->VariableTag##LocalTransformRevision != VariableTag##LastUpdateParentTransformRevision ||           \
            VariableTag##LocalTransformChangedSinceLastUpdate)                                                         \
        {                                                                                                              \
            /* TODO: We already had matrix representation of `parent->*WorldTransform` during                          \
                     `parent->Update*WorldTransformCache` call unless parent is a root node. Therefore,                \
                     we might try to optimize this method by introducing matrix caching. However, it would             \
                     slow down root node getters, because they would calculate unneeded matrix. */                     \
            VariableTag##WorldTransformCache = parent->VariableTag##WorldTransformCache * VariableTag##LocalTransform; \
            VariableTag##LastUpdateParentTransformRevision = parent->VariableTag##LocalTransformRevision;              \
            VariableTag##LocalTransformChangedSinceLastUpdate = false;                                                 \
            return true;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        return false;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    /* This component is a root node. */                                                                               \
                                                                                                                       \
    if (VariableTag##LocalTransformChangedSinceLastUpdate)                                                             \
    {                                                                                                                  \
        VariableTag##WorldTransformCache = VariableTag##LocalTransform;                                                \
        VariableTag##LocalTransformChangedSinceLastUpdate = false;                                                     \
        return true;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    return false

template <typename Transform>
bool TransformComponent<Transform>::UpdateLogicalWorldTransformCache (
    TransformWorldAccessor<Transform> &_accessor) const noexcept
{
    CACHE_UPDATE_METHOD (Logical, logical);
}

template <typename Transform>
bool TransformComponent<Transform>::UpdateVisualWorldTransformCache (
    TransformWorldAccessor<Transform> &_accessor) const noexcept
{
    CACHE_UPDATE_METHOD (Visual, visual);
}

template <typename Transform>
const typename TransformComponent<Transform>::Reflection &TransformComponent<Transform>::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        constexpr const char *NAME = [] () constexpr
        {
            if constexpr (std::is_same_v<Transform, Math::Transform2d>)
            {
                return "Transform2dComponent";
            }

            if constexpr (std::is_same_v<Transform, Math::Transform3d>)
            {
                return "Transform3dComponent";
            }

            return "TransformUnknownComponent";
        }();

        EMERGENCE_MAPPING_REGISTRATION_BEGIN_WITH_CUSTOM_NAME (TransformComponent, NAME);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parentObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (logicalLocalTransform);
        EMERGENCE_MAPPING_REGISTER_REGULAR (visualLocalTransform);
        EMERGENCE_MAPPING_REGISTER_REGULAR (visualTransformSyncNeeded);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lastObservedLogicalTransformRevision);
        EMERGENCE_MAPPING_REGISTER_REGULAR (logicalTransformLastObservationTimeNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (visualTransformLastSyncTimeNs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

template class TransformComponent<Math::Transform2d>;
template class TransformComponent<Math::Transform3d>;
} // namespace Emergence::Celerity
