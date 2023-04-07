#include <Platformer/Movement/CollisionShapeMovementContextComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const CollisionShapeMovementContextComponent::Reflection &CollisionShapeMovementContextComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionShapeMovementContextComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (supportedStates);
        EMERGENCE_MAPPING_REGISTER_REGULAR (useForGroundContactCheck);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
