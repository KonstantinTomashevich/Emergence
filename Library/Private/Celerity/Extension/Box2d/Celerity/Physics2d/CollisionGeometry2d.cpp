#include <Celerity/Physics2d/CollisionGeometry2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Line2d::Reflection &Line2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Line2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (start);
        EMERGENCE_MAPPING_REGISTER_REGULAR (end);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const CollisionGeometry2d::Reflection &CollisionGeometry2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionGeometry2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (boxHalfExtents);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (circleRadius);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (line);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
