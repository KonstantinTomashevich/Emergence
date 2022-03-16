#pragma once

#include <array>
#include <type_traits>

namespace Emergence::Math
{
class alignas (16) Matrix4x4f final
{
public:
    static const Matrix4x4f ZERO;
    static const Matrix4x4f IDENTITY;

    /// \details Special no-initialization constructor, used in algorithms to avoid unnecessary initialization.
    Matrix4x4f () noexcept;

    Matrix4x4f (const std::array<float, 16u> &_values) noexcept;

    Matrix4x4f (const Matrix4x4f &_other) noexcept;

    Matrix4x4f (Matrix4x4f &&_other) noexcept;

    ~Matrix4x4f () noexcept = default;

    Matrix4x4f &Transpose () noexcept;

    [[nodiscard]] Matrix4x4f CalculateInverse () const noexcept;

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
} // namespace Emergence::Math
