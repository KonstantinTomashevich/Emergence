#pragma once

#include <array>
#include <type_traits>

#include <Math/NoInitializationFlag.hpp>

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

    Matrix4x4f &operator*= (float _scalar) noexcept;

    Matrix4x4f &operator= (const Matrix4x4f &_other) noexcept;

    Matrix4x4f &operator= (Matrix4x4f &&_other) noexcept;

    using Column = float[4u];

    Column columns[4u];
};

/// \brief Special multiplication for transform matrices, that is slightly faster that usual multiplication.
Matrix4x4f MultiplyTransformMatrices (const Matrix4x4f &_left, const Matrix4x4f &_right) noexcept;
} // namespace Emergence::Math
