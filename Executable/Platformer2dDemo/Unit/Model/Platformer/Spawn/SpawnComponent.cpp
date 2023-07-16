#include <Platformer/Spawn/SpawnComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const SpawnComponent::Reflection &SpawnComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SpawnComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (prototypeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spawnDelayNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nextSpawnTimeNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxSpawnedCount);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentSpawnedCount);
        EMERGENCE_MAPPING_REGISTER_REGULAR (respawn);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
