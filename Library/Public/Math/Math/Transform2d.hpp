#pragma once

#include <type_traits>

#include <Math/Matrix3x3f.hpp>
#include <Math/NoInitializationFlag.hpp>
#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct alignas (float) Transform2d final
{
    Transform2d (const NoInitializationFlag &_guard) noexcept;

    Transform2d (const Vector2f &_translation = Vector2f::ZERO,
                 float _rotation = 0.0f,
                 const Vector2f &_scale = Vector2f::ONE) noexcept;

    Transform2d (const Matrix3x3f &_transformMatrix) noexcept;

    /// \warning This is just a shortcut for multiplication through matrices.
    ///          If you need to multiply several transforms, consider explicitly using matrices for performance.
    Transform2d operator* (const Transform2d &_other) const noexcept;

    Vector2f translation;
    float rotation;
    Vector2f scale;

    struct Reflection final
    {
        StandardLayout::FieldId translation;
        StandardLayout::FieldId rotation;
        StandardLayout::FieldId scale;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Transform2d>);
static_assert (std::is_trivially_move_constructible_v<Transform2d>);
static_assert (std::is_trivially_destructible_v<Transform2d>);
static_assert (std::is_trivially_copy_assignable_v<Transform2d>);
static_assert (std::is_trivially_move_assignable_v<Transform2d>);

bool NearlyEqual (const Transform2d &_first, const Transform2d &_second) noexcept;
} // namespace Emergence::Math
