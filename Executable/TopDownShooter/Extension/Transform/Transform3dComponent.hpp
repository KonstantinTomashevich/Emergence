#pragma once

#include <Celerity/Constants.hpp>

#include <Math/Transform3d.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence::Transform
{
class Transform3dWorldAccessor;

class Transform3dComponent final
{
public:
    Transform3dComponent () noexcept = default;

    Celerity::ObjectId GetObjectId () const noexcept;

    void SetObjectId (Celerity::ObjectId _objectId) noexcept;

    Celerity::ObjectId GetParentObjectId () const noexcept;

    void SetParentObjectId (Celerity::ObjectId _parentObjectId) noexcept;

    const Math::Transform3d &GetLogicalLocalTransform () const noexcept;

    void SetLogicalLocalTransform (const Math::Transform3d &_transform) noexcept;

    const Math::Transform3d &GetLogicalWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

    const Math::Transform3d &GetVisualLocalTransform () const noexcept;

    void SetVisualLocalTransform (const Math::Transform3d &_transform) noexcept;

    const Math::Transform3d &GetVisualWorldTransform (Transform3dWorldAccessor &_accessor) const noexcept;

private:
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

public:
    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId parentObjectId;
        Emergence::StandardLayout::FieldId logicalLocalTransform;
        Emergence::StandardLayout::FieldId visualLocalTransform;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Transform
