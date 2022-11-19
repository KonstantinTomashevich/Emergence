#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>

#include <Math/Matrix4x4f.hpp>
#include <Math/Transform3d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#if defined(__clang__)
// There is no align-mismatch check in old versions.
#    pragma clang diagnostic ignored "-Wunknown-warning-option"

// Alignment requirement should be satisfied because of matrix alignment, therefore CLang warnings are false positives.
#    pragma clang diagnostic ignored "-Walign-mismatch"
#endif

namespace Emergence::Math
{
static_assert (sizeof (Matrix4x4f) == sizeof (mat4));
static_assert (alignof (Matrix4x4f) == alignof (mat4));

const Matrix4x4f Matrix4x4f::ZERO {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

const Matrix4x4f Matrix4x4f::IDENTITY {
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Matrix4x4f::Matrix4x4f (const NoInitializationFlag & /*unused*/) noexcept
{
}

Matrix4x4f::Matrix4x4f (const std::array<float, 16u> &_values) noexcept
{
    memcpy (columns, &_values.front (), sizeof (columns));
}

Matrix4x4f::Matrix4x4f (const Transform3d &_transform) noexcept
{
    Matrix4x4f intermediate {NoInitializationFlag::Confirm ()};
    glm_translate_make (intermediate.columns, const_cast<float *> (_transform.translation.components));
    glm_quat_rotate (intermediate.columns, const_cast<float *> (_transform.rotation.components), columns);
    glm_scale (columns, const_cast<float *> (_transform.scale.components));
}

Matrix4x4f::Matrix4x4f (const Matrix4x4f &_other) noexcept
{
    glm_mat4_copy (const_cast<Column *> (_other.columns), columns);
}

Matrix4x4f::Matrix4x4f (Matrix4x4f &&_other) noexcept
    // NOLINTNEXTLINE(performance-move-constructor-init): We can not move anything, therefore we just copy.
    : Matrix4x4f (const_cast<const Matrix4x4f &> (_other))
{
}

Matrix4x4f &Matrix4x4f::Transpose () noexcept
{
    glm_mat4_transpose (columns);
    return *this;
}

Matrix4x4f Matrix4x4f::CalculateInverse () const noexcept
{
    Matrix4x4f result {NoInitializationFlag::Confirm ()};
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
    Matrix4x4f result {NoInitializationFlag::Confirm ()};
    glm_mat4_mul (const_cast<Column *> (columns), const_cast<Column *> (_other.columns), result.columns);
    return result;
}

Matrix4x4f Matrix4x4f::operator* (float _scalar) const noexcept
{
    Matrix4x4f result = *this;
    return result *= _scalar;
}

Vector4f Matrix4x4f::operator* (const Vector4f &_vector) const noexcept
{
    Vector4f result {NoInitializationFlag::Confirm ()};
    glm_mat4_mulv (const_cast<Column *> (columns), const_cast<float *> (_vector.components), result.components);
    return result;
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

const Matrix4x4f::Reflection &Matrix4x4f::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Matrix4x4f);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m00);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m01);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m02);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m03);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m10);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m11);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m12);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m13);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m20);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m21);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m22);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m23);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m30);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m31);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m33);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

Matrix4x4f MultiplyTransformMatrices (const Matrix4x4f &_left, const Matrix4x4f &_right) noexcept
{
    Matrix4x4f result {NoInitializationFlag::Confirm ()};
    glm_mul (const_cast<Matrix4x4f::Column *> (_left.columns), const_cast<Matrix4x4f::Column *> (_right.columns),
             result.columns);
    return result;
}
} // namespace Emergence::Math
