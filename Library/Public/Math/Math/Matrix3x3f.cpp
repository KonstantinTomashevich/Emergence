#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/affine2d.h>
#include <cglm/mat3.h>

#include <Math/Matrix3x3f.hpp>
#include <Math/Transform2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
static_assert (sizeof (Matrix3x3f) == sizeof (mat3));
static_assert (alignof (Matrix3x3f) == alignof (mat3));

const Matrix3x3f Matrix3x3f::ZERO {{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

const Matrix3x3f Matrix3x3f::IDENTITY {{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::columns initialization.
Matrix3x3f::Matrix3x3f (const NoInitializationFlag & /*unused*/) noexcept
{
}

Matrix3x3f::Matrix3x3f (const std::array<float, 9u> &_values) noexcept
{
    memcpy (columns, &_values.front (), sizeof (columns));
}

Matrix3x3f::Matrix3x3f (const Transform2d &_transform) noexcept
{
    glm_translate2d_make (columns, const_cast<float *> (_transform.translation.components));
    glm_rotate2d (columns, _transform.rotation);
    glm_scale2d (columns, const_cast<float *> (_transform.scale.components));
}

Matrix3x3f::Matrix3x3f (const Matrix3x3f &_other) noexcept
{
    glm_mat3_copy (const_cast<Column *> (_other.columns), columns);
}

Matrix3x3f::Matrix3x3f (Matrix3x3f &&_other) noexcept
    // NOLINTNEXTLINE(performance-move-constructor-init): We can not move anything, therefore we just copy.
    : Matrix3x3f (const_cast<const Matrix3x3f &> (_other))
{
}

Matrix3x3f &Matrix3x3f::Transpose () noexcept
{
    glm_mat3_transpose (columns);
    return *this;
}

Matrix3x3f Matrix3x3f::CalculateInverse () const noexcept
{
    Matrix3x3f result {NoInitializationFlag::Confirm ()};
    glm_mat3_inv (const_cast<Column *> (columns), result.columns);
    return result;
}

float Matrix3x3f::CalculateTrace () const noexcept
{
    return columns[0u][0u] + columns[1u][1u] + columns[2u][2u];
}

float Matrix3x3f::CalculateDeterminant () const noexcept
{
    return glm_mat3_det (const_cast<Column *> (columns));
}

Matrix3x3f Matrix3x3f::operator* (const Matrix3x3f &_other) const noexcept
{
    Matrix3x3f result {NoInitializationFlag::Confirm ()};
    glm_mat3_mul (const_cast<Column *> (columns), const_cast<Column *> (_other.columns), result.columns);
    return result;
}

Matrix3x3f Matrix3x3f::operator* (float _scalar) const noexcept
{
    Matrix3x3f result = *this;
    return result *= _scalar;
}

Vector3f Matrix3x3f::operator* (const Vector3f &_vector) const noexcept
{
    Vector3f result {NoInitializationFlag::Confirm ()};
    glm_mat3_mulv (const_cast<Column *> (columns), const_cast<float *> (_vector.components), result.components);
    return result;
}

Matrix3x3f &Matrix3x3f::operator*= (float _scalar) noexcept
{
    glm_mat3_scale (columns, _scalar);
    return *this;
}

Matrix3x3f &Matrix3x3f::operator= (const Matrix3x3f &_other) noexcept
{
    // We have trivial destructor, therefore we can just invoke constructor.
    new (this) Matrix3x3f (_other);
    return *this;
}

Matrix3x3f &Matrix3x3f::operator= (Matrix3x3f &&_other) noexcept
{
    // We have trivial destructor, therefore we can just invoke constructor.
    new (this) Matrix3x3f (_other);
    return *this;
}

const Matrix3x3f::Reflection &Matrix3x3f::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Matrix3x3f);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m00);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m01);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m02);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m10);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m11);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m12);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m20);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m21);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m22);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Math
