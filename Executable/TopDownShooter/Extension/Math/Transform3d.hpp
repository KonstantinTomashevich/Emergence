#pragma once

#include <type_traits>

#include <Math/Matrix4x4f.hpp>
#include <Math/NoInitializationFlag.hpp>
#include <Math/Quaternion.hpp>
#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct alignas (sizeof (float) * 4u) Transform3d final
{
    Transform3d (const NoInitializationFlag &_guard) noexcept;

    Transform3d (const Vector3f &_translation = Vector3f::ZERO,
                 const Quaternion &_rotation = Quaternion::IDENTITY,
                 const Vector3f &_scale = Vector3f::ONE) noexcept;

    Transform3d (const Matrix4x4f &_transformMatrix) noexcept;

    /// \warning This is just a shortcut for multiplication through matrices.
    ///          If you need to multiply several transforms, consider explicitly using matrices for performance.
    Transform3d operator* (const Transform3d &_other) const noexcept;

    // This order of fields is used to avoid alignment padding.

    Quaternion rotation;
    Vector3f translation;
    Vector3f scale;

    struct Reflection final
    {
        StandardLayout::FieldId rotation;
        StandardLayout::FieldId translation;
        StandardLayout::FieldId scale;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Transform3d>);
static_assert (std::is_trivially_move_constructible_v<Transform3d>);
static_assert (std::is_trivially_destructible_v<Transform3d>);
static_assert (std::is_trivially_copy_assignable_v<Transform3d>);
static_assert (std::is_trivially_move_assignable_v<Transform3d>);

bool NearlyEqual (const Transform3d &_first, const Transform3d &_second) noexcept;
} // namespace Emergence::Math
