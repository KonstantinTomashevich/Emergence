// We use CGLM for math (especially non-trivial one), because it is written in an optimized way (including SIMD usage).
// We use a lot of const casts, because CGLM doesn't use const keyword even when arguments are constant.
#include <cglm/affine.h>

#include <Math/Scalar.hpp>
#include <Math/Transform2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
Transform2d::Transform2d (const NoInitializationFlag &_guard) noexcept
    : translation (_guard),
      scale (_guard)
{
}

Transform2d::Transform2d (const Vector2f &_translation, float _rotation, const Vector2f &_scale) noexcept
    : translation (_translation),
      rotation (_rotation),
      scale (_scale)
{
}

Transform2d::Transform2d (const Matrix3x3f &_transformMatrix) noexcept
    : translation (NoInitializationFlag::Confirm ()),
      scale (NoInitializationFlag::Confirm ())
{
    translation.x = _transformMatrix.columns[2][0];
    translation.y = _transformMatrix.columns[2][1];

    rotation = atan2f (_transformMatrix.columns[0][1], _transformMatrix.columns[0][0]);

    scale.x = sqrtf (_transformMatrix.columns[0][0] * _transformMatrix.columns[0][0] +
                     _transformMatrix.columns[0][1] * _transformMatrix.columns[0][1]);

    scale.y = sqrtf (_transformMatrix.columns[1][0] * _transformMatrix.columns[1][0] +
                     _transformMatrix.columns[1][1] * _transformMatrix.columns[1][1]);
}

Transform2d Transform2d::operator* (const Transform2d &_other) const noexcept
{
    return Matrix3x3f (*this) * Matrix3x3f (_other);
}

const Transform2d::Reflection &Transform2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Transform2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (translation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rotation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (scale);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

bool NearlyEqual (const Transform2d &_first, const Transform2d &_second) noexcept
{
    return NearlyEqual (_first.translation, _second.translation) && NearlyEqual (_first.rotation, _second.rotation) &&
           NearlyEqual (_first.scale, _second.scale);
}
} // namespace Emergence::Math
