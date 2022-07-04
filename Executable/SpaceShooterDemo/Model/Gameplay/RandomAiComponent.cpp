#include <Gameplay/RandomAiComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const RandomAiComponent::Reflection &RandomAiComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RandomAiComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentTargetPoint);
        EMERGENCE_MAPPING_REGISTER_REGULAR (chaseTargetPointUntilNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (averageShotsPerS);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
