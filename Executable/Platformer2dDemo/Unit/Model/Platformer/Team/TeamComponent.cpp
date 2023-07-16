#include <Platformer/Team/TeamComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const TeamComponent::Reflection &TeamComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TeamComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (teamId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
