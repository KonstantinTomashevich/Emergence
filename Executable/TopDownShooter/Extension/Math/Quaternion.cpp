// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>

#include <Math/Quaternion.hpp>

namespace Emergence::Math
{
static_assert (sizeof (Quaternion) == sizeof (versor));
static_assert (alignof (Quaternion) == alignof (versor));

const Quaternion Quaternion::IDENTITY {0.0f, 0.0f, 0.0f, 1.0f};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Quaternion::Quaternion (const NoInitializationFlag & /*unused*/) noexcept
{
}

Quaternion::Quaternion (float _x, float _y, float _z, float _w) noexcept : x (_x), y (_y), z (_z), w (_w)
{
}

Quaternion::Quaternion (float _angle, const Vector3f &_axis) noexcept
{
    glm_quatv (components, _angle, const_cast<float *> (_axis.components));
}

Quaternion::Quaternion (const Vector3f &_eulerRad) noexcept
{
    mat4 rotationMatrix;
    // Unfortunately, const is not used in CGLM.
    glm_euler_xyz (const_cast<float *> (_eulerRad.components), rotationMatrix);
    glm_mat4_quat (rotationMatrix, components);
}

Quaternion::Quaternion (const Vector3f &_from, const Vector3f &_to) noexcept
{
    glm_quat_from_vecs (const_cast<float *> (_from.components), const_cast<float *> (_to.components), components);
}

Quaternion &Quaternion::Normalize () noexcept
{
    glm_quat_normalize (components);
    return *this;
}

Quaternion Quaternion::CalculateConjugate () const noexcept
{
    Quaternion conjugate {NoInitializationFlag::Confirm ()};
    glm_quat_conjugate (const_cast<float *> (components), conjugate.components);
    return conjugate;
}

Quaternion Quaternion::CalculateInverse () const noexcept
{
    Quaternion inverse {NoInitializationFlag::Confirm ()};
    glm_quat_inv (const_cast<float *> (components), inverse.components);
    return inverse;
}

Vector3f Quaternion::CalculateEuler () const noexcept
{
    Vector3f result {NoInitializationFlag::Confirm ()};
    mat4 rotationMatrix;

    glm_quat_mat4 (const_cast<float *> (components), rotationMatrix);
    glm_euler_angles (rotationMatrix, result.components);
    return result;
}

#define IMPLEMENT_OPERATOR(Operator, CGLMName)                                                                         \
    Quaternion Quaternion::operator Operator (const Quaternion &_other) const noexcept                                 \
    {                                                                                                                  \
        Quaternion result {NoInitializationFlag::Confirm ()};                                                          \
        glm_quat_##CGLMName (const_cast<float *> (components), const_cast<float *> (_other.components),                \
                             result.components);                                                                       \
        return result;                                                                                                 \
    }

#define IMPLEMENT_ASSIGNMENT_OPERATOR(Operator, CGLMName)                                                              \
    Quaternion &Quaternion::operator Operator##= (const Quaternion &_other) noexcept                                   \
    {                                                                                                                  \
        glm_quat_##CGLMName (const_cast<float *> (components), const_cast<float *> (_other.components), components);   \
        return *this;                                                                                                  \
    }

IMPLEMENT_OPERATOR (+, add)
IMPLEMENT_ASSIGNMENT_OPERATOR (+, add)
IMPLEMENT_OPERATOR (-, sub)
IMPLEMENT_ASSIGNMENT_OPERATOR (-, sub)
IMPLEMENT_OPERATOR (*, mul)
IMPLEMENT_ASSIGNMENT_OPERATOR (*, mul)

Quaternion Lerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept
{
    Quaternion result {NoInitializationFlag::Confirm ()};
    glm_quat_lerp (const_cast<float *> (_begin.components), const_cast<float *> (_end.components), _t,
                   result.components);
    return result;
}

Quaternion NLerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept
{
    Quaternion result {NoInitializationFlag::Confirm ()};
    glm_quat_nlerp (const_cast<float *> (_begin.components), const_cast<float *> (_end.components), _t,
                    result.components);
    return result;
}

Quaternion SLerp (const Quaternion &_begin, const Quaternion &_end, float _t) noexcept
{
    Quaternion result {NoInitializationFlag::Confirm ()};
    glm_quat_slerp (const_cast<float *> (_begin.components), const_cast<float *> (_end.components), _t,
                    result.components);
    return result;
}

Quaternion ForwardRotation (const Vector3f &_forward, const Vector3f &_up) noexcept
{
    Quaternion result {NoInitializationFlag::Confirm ()};
    glm_quat_for (const_cast<float *> (_forward.components), const_cast<float *> (_up.components), result.components);
    return result;
}

Vector3f Rotate (const Vector3f &_vector, const Quaternion &_rotation) noexcept
{
    Vector3f result {NoInitializationFlag::Confirm ()};
    glm_quat_rotatev (const_cast<float *> (_rotation.components), const_cast<float *> (_vector.components),
                      result.components);
    return result;
}
} // namespace Emergence::Math
