#include <Celerity/Physics/CollisionGeometry.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const CollisionGeometry::Reflection &CollisionGeometry::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionGeometry)
        EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR (type)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (boxHalfExtents)
        EMERGENCE_MAPPING_REGISTER_REGULAR (sphereRadius)
        EMERGENCE_MAPPING_REGISTER_REGULAR (capsuleRadius)
        EMERGENCE_MAPPING_REGISTER_REGULAR (capsuleHalfHeight)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Celerity
