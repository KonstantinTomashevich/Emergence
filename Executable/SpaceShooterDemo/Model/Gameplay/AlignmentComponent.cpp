#include <Gameplay/AlignmentComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const AlignmentComponent::Reflection &AlignmentComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AlignmentComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (playerId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
