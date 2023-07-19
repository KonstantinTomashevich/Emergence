#pragma once

#include <MathApi.hpp>

#include <type_traits>

#include <Math/NoInitializationFlag.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct alignas (float) MathApi Vector4f final
{
    static const Vector4f ZERO;
    static const Vector4f ONE;

    Vector4f (const NoInitializationFlag & /*unused*/) noexcept;

    Vector4f (float _x, float _y, float _z, float _w) noexcept;

    /// \warning If you need length just for comparison, consider using ::LengthSquared to avoid costly sqrt call.
    [[nodiscard]] float Length () const noexcept;

    [[nodiscard]] float LengthSquared () const noexcept;

    Vector4f &Normalize () noexcept;

    Vector4f &NormalizeSafe () noexcept;

    Vector4f operator+ (const Vector4f &_other) const noexcept;

    Vector4f &operator+= (const Vector4f &_other) noexcept;

    Vector4f operator- (const Vector4f &_other) const noexcept;

    Vector4f &operator-= (const Vector4f &_other) noexcept;

    Vector4f operator* (const Vector4f &_other) const noexcept;

    Vector4f &operator*= (const Vector4f &_other) noexcept;

    Vector4f operator/ (const Vector4f &_other) const noexcept;

    Vector4f &operator/= (const Vector4f &_other) noexcept;

    Vector4f operator+ (float _scalar) const noexcept;

    Vector4f &operator+= (float _scalar) noexcept;

    Vector4f operator- (float _scalar) const noexcept;

    Vector4f &operator-= (float _scalar) noexcept;

    Vector4f operator* (float _scalar) const noexcept;

    Vector4f &operator*= (float _scalar) noexcept;

    Vector4f operator/ (float _scalar) const noexcept;

    Vector4f &operator/= (float _scalar) noexcept;

    Vector4f operator- () const noexcept;

    union
    {
        float components[3u];
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    struct MathApi Reflection final
    {
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId z;
        StandardLayout::FieldId w;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_copy_constructible_v<Vector4f>);
static_assert (std::is_trivially_move_constructible_v<Vector4f>);
static_assert (std::is_trivially_destructible_v<Vector4f>);
static_assert (std::is_trivially_copy_assignable_v<Vector4f>);
static_assert (std::is_trivially_move_assignable_v<Vector4f>);

MathApi float Dot (const Vector4f &_first, const Vector4f &_second) noexcept;

MathApi Vector4f Lerp (const Vector4f &_begin, const Vector4f &_end, float _t) noexcept;

MathApi bool NearlyEqual (const Vector4f &_first, const Vector4f &_second) noexcept;
} // namespace Emergence::Math
