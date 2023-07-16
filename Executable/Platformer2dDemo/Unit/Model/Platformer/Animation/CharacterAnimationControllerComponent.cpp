#include <Platformer/Animation/CharacterAnimationControllerComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const CharacterAnimationControllerComponent::Reflection &CharacterAnimationControllerComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CharacterAnimationControllerComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spriteId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (animationConfigurationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (direction);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
