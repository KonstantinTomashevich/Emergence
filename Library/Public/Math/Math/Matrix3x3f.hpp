#pragma once

#include <array>
#include <type_traits>

#include <Math/NoInitializationFlag.hpp>

namespace Emergence::Math
{
struct Transform2d;

struct Matrix3x3f final
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

    Matrix3x3f &operator*= (float _scalar) noexcept;

    Matrix3x3f &operator= (const Matrix3x3f &_other) noexcept;

    Matrix3x3f &operator= (Matrix3x3f &&_other) noexcept;

    using Column = float[3u];

    Column columns[3u];
};
} // namespace Emergence::Math
