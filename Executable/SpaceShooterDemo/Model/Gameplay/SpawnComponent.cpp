#include <Gameplay/SpawnComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const SpawnComponent::Reflection &SpawnComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SpawnComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectToSpawnId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spawnCoolDownNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spawnCoolingDownUntilNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxSpawnedObjects);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spawnedObjects);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
