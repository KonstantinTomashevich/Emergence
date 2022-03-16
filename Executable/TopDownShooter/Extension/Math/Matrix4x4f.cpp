#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/mat4.h>

#include <Math/Matrix4x4f.hpp>

namespace Emergence::Math
{
static_assert (sizeof (Matrix4x4f) == sizeof (mat4));
static_assert (alignof (Matrix4x4f) == alignof (mat4));

const Matrix4x4f Matrix4x4f::ZERO {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

const Matrix4x4f Matrix4x4f::IDENTITY {
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Matrix4x4f::Matrix4x4f () noexcept
{
}

Matrix4x4f::Matrix4x4f (const std::array<float, 16u> &_values) noexcept
{
    memcpy (columns, &_values.front (), sizeof (columns));
}

Matrix4x4f::Matrix4x4f (const Matrix4x4f &_other) noexcept
{
    // Alignment requirement should be satisfied because of matrix alignment, therefore CLion warning is false positive.
    glm_mat4_copy (const_cast<Column *> (_other.columns), columns);
}

// NOLINTNEXTLINE(performance-move-constructor-init): We can not move anything, therefore we just copy.
Matrix4x4f::Matrix4x4f (Matrix4x4f &&_other) noexcept : Matrix4x4f (const_cast<const Matrix4x4f &> (_other))
{
}

Matrix4x4f &Matrix4x4f::Transpose () noexcept
{
    glm_mat4_transpose (columns);
    return *this;
}

Matrix4x4f Matrix4x4f::CalculateInverse () const noexcept
{
    Matrix4x4f result;
    glm_mat4_inv (const_cast<Column *> (columns), result.columns);
    return result;
}

float Matrix4x4f::CalculateTrace () const noexcept
{
    return columns[0u][0u] + columns[1u][1u] + columns[2u][2u] + columns[3u][3u];
}

float Matrix4x4f::CalculateDeterminant () const noexcept
{
    return glm_mat4_det (const_cast<Column *> (columns));
}

Matrix4x4f Matrix4x4f::operator* (const Matrix4x4f &_other) const noexcept
{
    Matrix4x4f result;
    glm_mat4_mul (const_cast<Column *> (columns), const_cast<Column *> (_other.columns), result.columns);
    return result;
}

Matrix4x4f Matrix4x4f::operator* (float _scalar) const noexcept
{
    Matrix4x4f result = *this;
    return result *= _scalar;
}

Matrix4x4f &Matrix4x4f::operator*= (float _scalar) noexcept
{
    glm_mat4_scale (columns, _scalar);
    return *this;
}

Matrix4x4f &Matrix4x4f::operator= (const Matrix4x4f &_other) noexcept
{
    // We have trivial destructor, therefore we can just invoke constructor.
    new (this) Matrix4x4f (_other);
    return *this;
}

Matrix4x4f &Matrix4x4f::operator= (Matrix4x4f &&_other) noexcept
{
    // We have trivial destructor, therefore we can just invoke constructor.
    new (this) Matrix4x4f (_other);
    return *this;
}
} // namespace Emergence::Math
