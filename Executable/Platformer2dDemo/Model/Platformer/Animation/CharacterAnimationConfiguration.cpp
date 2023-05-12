#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const CharacterAnimationConfiguration::Reflection &CharacterAnimationConfiguration::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CharacterAnimationConfiguration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (crouchAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fallAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (idleAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (jumpAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rollAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (runAnimationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (slideAnimationId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
