#pragma once

#include <MathApi.hpp>

#include <type_traits>

#include <Math/NoInitializationFlag.hpp>
#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct MathApi alignas (sizeof (float) * 4u) Quaternion final
{
    static const Quaternion IDENTITY;

    Quaternion (const NoInitializationFlag & /*unused*/) noexcept;

    Quaternion (float _x, float _y, float _z, float _w) noexcept;

    /// \invariant `_axis` length is 1.0.
    Quaternion (float _angle, const Vector3f &_axis) noexcept;

    Quaternion (const Vector3f &_eulerRad) noexcept;

    /// \invariant `_from` and `_to` lengths are 1.0.
    Quaternion (const Vector3f &_from, const Vector3f &_to) noexcept;

    Quaternion &Normalize () noexcept;

    /// \return Quaternion with negated imaginary part.
    [[nodiscard]] Quaternion CalculateConjugate () const noexcept;

    /// \details Quaternion * InverseQuaternion = Identity
    [[nodiscard]] Quaternion CalculateInverse () const noexcept;

    /// \return Rotation of this quaternion, represented using euler angles.
    [[nodiscard]] Vector3f CalculateEuler () const noexcept;

    Quaternion operator+ (const Quaternion &_other) const noexcept;

    Quaternion &operator+= (const Quaternion &_other) noexcept;

    Quaternion operator- (const Quaternion &_other) const noexcept;

    Quaternion &operator-= (const Quaternion &_other) noexcept;

    Quaternion operator* (const Quaternion &_other) const noexcept;

    Quaternion &operator*= (const Quaternion &_other) noexcept;

    union
    {
        float components[4u];

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

static_assert (std::is_trivially_copy_constructible_v<Quaternion>);
static_assert (std::is_trivially_move_constructible_v<Quaternion>);
static_assert (std::is_trivially_destructible_v<Quaternion>);
static_assert (std::is_trivially_copy_assignable_v<Quaternion>);
static_assert (std::is_trivially_move_assignable_v<Quaternion>);

MathApi Quaternion Lerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept;

MathApi Quaternion NLerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept;

MathApi Quaternion SLerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept;

MathApi Quaternion ForwardRotation (const Vector3f &_forward, const Vector3f &_up = Vector3f::UP) noexcept;

MathApi Vector3f Rotate (const Vector3f &_vector, const Quaternion &_rotation) noexcept;

MathApi bool NearlyEqual (const Quaternion &_first, const Quaternion &_second) noexcept;
} // namespace Emergence::Math
