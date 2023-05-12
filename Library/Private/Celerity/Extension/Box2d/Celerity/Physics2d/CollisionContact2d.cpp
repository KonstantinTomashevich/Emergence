#include <Celerity/Physics2d/CollisionContact2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
bool CollisionContact2d::operator== (const CollisionContact2d &_other) const
{
    bool fieldsEqual = collisionContactId == _other.collisionContactId && objectId == _other.objectId &&
                       otherObjectId == _other.otherObjectId && shapeId == _other.shapeId &&
                       otherShapeId == _other.otherShapeId && Math::NearlyEqual (normal, _other.normal) &&
                       points.GetCount () == _other.points.GetCount ();

    if (fieldsEqual)
    {
        for (size_t index = 0u; index < points.GetCount (); ++index)
        {
            fieldsEqual |= Math::NearlyEqual (points[index], _other.points[index]);
        }
    }

    return fieldsEqual;
}

const CollisionContact2d::Reflection &CollisionContact2d::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionContact2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (collisionContactId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (otherObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (otherShapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (normal);
        EMERGENCE_MAPPING_REGISTER_REGULAR (points);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
