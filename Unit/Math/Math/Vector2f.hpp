#pragma once

#include <MathApi.hpp>

#include <type_traits>

#include <Math/NoInitializationFlag.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct alignas (float) MathApi Vector2f final
{
    static const Vector2f ZERO;
    static const Vector2f ONE;

    static const Vector2f RIGHT;
    static const Vector2f LEFT;

    static const Vector2f UP;
    static const Vector2f DOWN;

    Vector2f (const NoInitializationFlag & /*unused*/) noexcept;

    Vector2f (float _x, float _y) noexcept;

    /// \warning If you need length just for comparison, consider using ::LengthSquared to avoid costly sqrt call.
    [[nodiscard]] float Length () const noexcept;

    [[nodiscard]] float LengthSquared () const noexcept;

    Vector2f &Normalize () noexcept;

    Vector2f &NormalizeSafe () noexcept;

    Vector2f operator+ (const Vector2f &_other) const noexcept;

    Vector2f &operator+= (const Vector2f &_other) noexcept;

    Vector2f operator- (const Vector2f &_other) const noexcept;

    Vector2f &operator-= (const Vector2f &_other) noexcept;

    Vector2f operator* (const Vector2f &_other) const noexcept;

    Vector2f &operator*= (const Vector2f &_other) noexcept;

    Vector2f operator/ (const Vector2f &_other) const noexcept;

    Vector2f &operator/= (const Vector2f &_other) noexcept;

    Vector2f operator+ (float _scalar) const noexcept;

    Vector2f &operator+= (float _scalar) noexcept;

    Vector2f operator- (float _scalar) const noexcept;

    Vector2f &operator-= (float _scalar) noexcept;

    Vector2f operator* (float _scalar) const noexcept;

    Vector2f &operator*= (float _scalar) noexcept;

    Vector2f operator/ (float _scalar) const noexcept;

    Vector2f &operator/= (float _scalar) noexcept;

    Vector2f operator- () const noexcept;

    union
    {
        float components[2u];
        struct
        {
            float x;
            float y;
        };
    };

    struct MathApi Reflection final
    {
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Vector2f>);
static_assert (std::is_trivially_move_constructible_v<Vector2f>);
static_assert (std::is_trivially_destructible_v<Vector2f>);
static_assert (std::is_trivially_copy_assignable_v<Vector2f>);
static_assert (std::is_trivially_move_assignable_v<Vector2f>);

MathApi float Dot (const Vector2f &_first, const Vector2f &_second) noexcept;

MathApi float SignedAngle (const Vector2f &_from, const Vector2f &_to) noexcept;

MathApi Vector2f Lerp (const Vector2f &_begin, const Vector2f &_end, float _t) noexcept;

MathApi bool NearlyEqual (const Vector2f &_first, const Vector2f &_second) noexcept;

MathApi Vector2f Rotate (const Vector2f &_vector, float _angleRad) noexcept;
} // namespace Emergence::Math
