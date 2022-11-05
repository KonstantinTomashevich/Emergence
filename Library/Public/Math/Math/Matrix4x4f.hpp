#pragma once

#include <array>

#include <Math/NoInitializationFlag.hpp>
#include <Math/Vector4f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct Transform3d;

struct alignas (sizeof (float) * 4u) Matrix4x4f final
{
    static const Matrix4x4f ZERO;
    static const Matrix4x4f IDENTITY;

    Matrix4x4f (const NoInitializationFlag & /*unused*/) noexcept;

    Matrix4x4f (const std::array<float, 16u> &_values) noexcept;

    Matrix4x4f (const Transform3d &_transform) noexcept;

    Matrix4x4f (const Matrix4x4f &_other) noexcept;

    Matrix4x4f (Matrix4x4f &&_other) noexcept;

    ~Matrix4x4f () noexcept = default;

    /// \details Transposition "rotates" matrix by makings its rows columns.
    Matrix4x4f &Transpose () noexcept;

    /// \details Matrix * InverseMatrix = Identity (if inverse exists)
    [[nodiscard]] Matrix4x4f CalculateInverse () const noexcept;

    /// \return Sum of diagonal elements.
    [[nodiscard]] float CalculateTrace () const noexcept;

    [[nodiscard]] float CalculateDeterminant () const noexcept;

    Matrix4x4f operator* (const Matrix4x4f &_other) const noexcept;

    Matrix4x4f operator* (float _scalar) const noexcept;

    Vector4f operator* (const Vector4f &_vector) const noexcept;

    Matrix4x4f &operator*= (float _scalar) noexcept;

    Matrix4x4f &operator= (const Matrix4x4f &_other) noexcept;

    Matrix4x4f &operator= (Matrix4x4f &&_other) noexcept;

    using Column = float[4u];

    union
    {
        Column columns[4u];

        struct
        {
            float m00;
            float m01;
            float m02;
            float m03;
            float m10;
            float m11;
            float m12;
            float m13;
            float m20;
            float m21;
            float m22;
            float m23;
            float m30;
            float m31;
            float m32;
            float m33;
        };
    };

    struct Reflection final
    {
        StandardLayout::FieldId m00;
        StandardLayout::FieldId m01;
        StandardLayout::FieldId m02;
        StandardLayout::FieldId m03;
        StandardLayout::FieldId m10;
        StandardLayout::FieldId m11;
        StandardLayout::FieldId m12;
        StandardLayout::FieldId m13;
        StandardLayout::FieldId m20;
        StandardLayout::FieldId m21;
        StandardLayout::FieldId m22;
        StandardLayout::FieldId m23;
        StandardLayout::FieldId m30;
        StandardLayout::FieldId m31;
        StandardLayout::FieldId m32;
        StandardLayout::FieldId m33;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Special multiplication for transform matrices, that is slightly faster that usual multiplication.
Matrix4x4f MultiplyTransformMatrices (const Matrix4x4f &_left, const Matrix4x4f &_right) noexcept;
} // namespace Emergence::Math
