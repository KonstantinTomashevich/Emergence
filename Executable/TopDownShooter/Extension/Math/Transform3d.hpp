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

    Transform3d (const Vector3f &_translation, const Quaternion &_rotation, const Vector3f &_scale) noexcept;

    Transform3d (const Matrix4x4f &_transformMatrix) noexcept;

    /// \warning This is just a shortcut for multiplication through matrices.
    ///          If you need to multiply several transforms, consider explicitly using matrices for performance.
    Transform3d operator* (const Transform3d &_other) noexcept;

    Vector3f translation;
    Quaternion rotation;
    Vector3f scale;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId translation;
        Emergence::StandardLayout::FieldId rotation;
        Emergence::StandardLayout::FieldId scale;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Transform3d>);
static_assert (std::is_trivially_move_constructible_v<Transform3d>);
static_assert (std::is_trivially_destructible_v<Transform3d>);
static_assert (std::is_trivially_copy_assignable_v<Transform3d>);
static_assert (std::is_trivially_move_assignable_v<Transform3d>);
} // namespace Emergence::Math
