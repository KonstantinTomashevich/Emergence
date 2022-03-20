// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/affine.h>

#include <Math/Transform3d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
Transform3d::Transform3d (const NoInitializationFlag &_guard) noexcept
    : translation (_guard),
      rotation (_guard),
      scale (_guard)
{
}

Transform3d::Transform3d (const Vector3f &_translation, const Quaternion &_rotation, const Vector3f &_scale) noexcept
    : translation (_translation),
      rotation (_rotation),
      scale (_scale)
{
}

Transform3d::Transform3d (const Matrix4x4f &_transformMatrix) noexcept
    : translation (NoInitializationFlag::Confirm ()),
      rotation (NoInitializationFlag::Confirm ()),
      scale (NoInitializationFlag::Confirm ())
{
    Matrix4x4f rotationMatrix {NoInitializationFlag::Confirm ()};
    glm_decompose (const_cast<Matrix4x4f::Column *> (_transformMatrix.columns), translation.components,
                   rotationMatrix.columns, scale.components);
    glm_mat4_quat (rotationMatrix.columns, rotation.components);
}

Transform3d Transform3d::operator* (const Transform3d &_other) noexcept
{
    return MultiplyTransformMatrices (Matrix4x4f (*this), Matrix4x4f (_other));
}

const Transform3d::Reflection &Transform3d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Transform3d)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (translation)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (rotation)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (scale)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Math
