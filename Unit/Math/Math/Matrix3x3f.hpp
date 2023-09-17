#pragma once

#include <MathApi.hpp>

#include <array>

#include <Math/NoInitializationFlag.hpp>
#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct Transform2d;

struct MathApi Matrix3x3f final
{
    static const Matrix3x3f ZERO;
    static const Matrix3x3f IDENTITY;

    Matrix3x3f (const NoInitializationFlag & /*unused*/) noexcept;

    Matrix3x3f (const std::array<float, 9u> &_values) noexcept;

    Matrix3x3f (const Transform2d &_transform) noexcept;

    Matrix3x3f (const Matrix3x3f &_other) noexcept;

    Matrix3x3f (Matrix3x3f &&_other) noexcept;

    ~Matrix3x3f () noexcept = default;

    /// \details Transposition "rotates" matrix by makings its rows columns.
    Matrix3x3f &Transpose () noexcept;

    /// \details Matrix * InverseMatrix = Identity (if inverse exists)
    [[nodiscard]] Matrix3x3f CalculateInverse () const noexcept;

    /// \return Sum of diagonal elements.
    [[nodiscard]] float CalculateTrace () const noexcept;

    [[nodiscard]] float CalculateDeterminant () const noexcept;

    Matrix3x3f operator* (const Matrix3x3f &_other) const noexcept;

    Matrix3x3f operator* (float _scalar) const noexcept;

    Vector3f operator* (const Vector3f &_vector) const noexcept;

    Matrix3x3f &operator*= (float _scalar) noexcept;

    Matrix3x3f &operator= (const Matrix3x3f &_other) noexcept;

    Matrix3x3f &operator= (Matrix3x3f &&_other) noexcept;

    using Column = float[3u];

    union
    {
        Column columns[3u];

        struct
        {
            float m00;
            float m01;
            float m02;
            float m10;
            float m11;
            float m12;
            float m20;
            float m21;
            float m22;
        };
    };

    struct MathApi Reflection final
    {
        StandardLayout::FieldId m00;
        StandardLayout::FieldId m01;
        StandardLayout::FieldId m02;
        StandardLayout::FieldId m10;
        StandardLayout::FieldId m11;
        StandardLayout::FieldId m12;
        StandardLayout::FieldId m20;
        StandardLayout::FieldId m21;
        StandardLayout::FieldId m22;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Math
