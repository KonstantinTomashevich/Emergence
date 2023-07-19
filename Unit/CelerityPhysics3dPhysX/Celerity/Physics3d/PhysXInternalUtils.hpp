#pragma once

#include <API/Common/MuteWarnings.hpp>

BEGIN_MUTING_WARNINGS
#include <foundation/PxQuat.h>
#include <foundation/PxVec3.h>
END_MUTING_WARNINGS

#include <Math/Quaternion.hpp>
#include <Math/Transform3d.hpp>
#include <Math/Vector3f.hpp>

namespace Emergence::Celerity
{
inline physx::PxVec3 ToPhysX (const Math::Vector3f &_vector) noexcept
{
    return {_vector.x, _vector.y, _vector.z};
}

inline physx::PxQuat ToPhysX (const Math::Quaternion &_quaternion) noexcept
{
    return {_quaternion.x, _quaternion.y, _quaternion.z, _quaternion.w};
}

inline Math::Vector3f FromPhysX (const physx::PxVec3 &_vector) noexcept
{
    return {_vector.x, _vector.y, _vector.z};
}

inline Math::Quaternion FromPhysX (const physx::PxQuat &_quaternion) noexcept
{
    return {_quaternion.x, _quaternion.y, _quaternion.z, _quaternion.w};
}
} // namespace Emergence::Celerity
