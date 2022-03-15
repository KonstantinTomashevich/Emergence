#pragma once

#include <type_traits>

#include <Math/Constants.hpp>

namespace Emergence::Math
{
class Vector3f final
{
public:
    static const Vector3f ZERO;

    static const Vector3f RIGHT;
    static const Vector3f LEFT;

    static const Vector3f UP;
    static const Vector3f DOWN;

    static const Vector3f FORWARD;
    static const Vector3f BACKWARD;

    /// \details Special no-initialization constructor, used in algorithms to avoid unnecessary initialization.
    Vector3f () noexcept;

    Vector3f (float _x, float _y, float _z) noexcept;

    [[nodiscard]] float Length () const noexcept;

    [[nodiscard]] float LengthSquared () const noexcept;

    Vector3f &Normalize () noexcept;

    Vector3f &NormalizeSafe (float _epsilon = EPSILON) noexcept;

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
};

static_assert (std::is_trivially_copy_constructible_v<Vector3f>);
static_assert (std::is_trivially_move_constructible_v<Vector3f>);
static_assert (std::is_trivially_destructible_v<Vector3f>);
static_assert (std::is_trivially_copy_assignable_v<Vector3f>);
static_assert (std::is_trivially_move_assignable_v<Vector3f>);

float Dot (const Vector3f &_first, const Vector3f &_second) noexcept;

Vector3f Cross (const Vector3f &_first, const Vector3f &_second) noexcept;

float UnsignedAngle (const Vector3f &_from, const Vector3f &_to) noexcept;

float SignedAngle (const Vector3f &_from, const Vector3f &_to, const Vector3f &_axis = Vector3f::UP) noexcept;

Vector3f Lerp (const Vector3f &_begin, const Vector3f &_end, float _t) noexcept;
} // namespace Emergence::Math
