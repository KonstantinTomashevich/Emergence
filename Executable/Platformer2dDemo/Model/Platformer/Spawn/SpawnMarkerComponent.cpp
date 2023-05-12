#include <Platformer/Spawn/SpawnMarkerComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const SpawnMarkerComponent::Reflection &SpawnMarkerComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SpawnMarkerComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spawnObjectId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
