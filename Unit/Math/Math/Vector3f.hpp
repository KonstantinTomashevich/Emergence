#pragma once

#include <MathApi.hpp>

#include <type_traits>

#include <Math/NoInitializationFlag.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct alignas (float) MathApi Vector3f final
{
    static const Vector3f ZERO;
    static const Vector3f ONE;

    static const Vector3f RIGHT;
    static const Vector3f LEFT;

    static const Vector3f UP;
    static const Vector3f DOWN;

    static const Vector3f FORWARD;
    static const Vector3f BACKWARD;

    Vector3f (const NoInitializationFlag & /*unused*/) noexcept;

    Vector3f (float _x, float _y, float _z) noexcept;

    /// \warning If you need length just for comparison, consider using ::LengthSquared to avoid costly sqrt call.
    [[nodiscard]] float Length () const noexcept;

    [[nodiscard]] float LengthSquared () const noexcept;

    Vector3f &Normalize () noexcept;

    Vector3f &NormalizeSafe () noexcept;

    Vector3f operator+ (const Vector3f &_other) const noexcept;

    Vector3f &operator+= (const Vector3f &_other) noexcept;

    Vector3f operator- (const Vector3f &_other) const noexcept;

    Vector3f &operator-= (const Vector3f &_other) noexcept;

    Vector3f operator* (const Vector3f &_other) const noexcept;

    Vector3f &operator*= (const Vector3f &_other) noexcept;

    Vector3f operator/ (const Vector3f &_other) const noexcept;

    Vector3f &operator/= (const Vector3f &_other) noexcept;

    Vector3f operator+ (float _scalar) const noexcept;

    Vector3f &operator+= (float _scalar) noexcept;

    Vector3f operator- (float _scalar) const noexcept;

    Vector3f &operator-= (float _scalar) noexcept;

    Vector3f operator* (float _scalar) const noexcept;

    Vector3f &operator*= (float _scalar) noexcept;

    Vector3f operator/ (float _scalar) const noexcept;

    Vector3f &operator/= (float _scalar) noexcept;

    Vector3f operator- () const noexcept;

    union
    {
        float components[3u];
        struct
        {
            float x;
            float y;
            float z;
        };
    };

    struct MathApi Reflection final
    {
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId z;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Vector3f>);
static_assert (std::is_trivially_move_constructible_v<Vector3f>);
static_assert (std::is_trivially_destructible_v<Vector3f>);
static_assert (std::is_trivially_copy_assignable_v<Vector3f>);
static_assert (std::is_trivially_move_assignable_v<Vector3f>);

MathApi float Dot (const Vector3f &_first, const Vector3f &_second) noexcept;

MathApi Vector3f Cross (const Vector3f &_first, const Vector3f &_second) noexcept;

MathApi float UnsignedAngle (const Vector3f &_from, const Vector3f &_to) noexcept;

MathApi float SignedAngle (const Vector3f &_from, const Vector3f &_to, const Vector3f &_axis = Vector3f::UP) noexcept;

MathApi Vector3f Lerp (const Vector3f &_begin, const Vector3f &_end, float _t) noexcept;

MathApi bool NearlyEqual (const Vector3f &_first, const Vector3f &_second) noexcept;
} // namespace Emergence::Math
