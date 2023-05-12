#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2dUvAnimationComponent::Reflection &Sprite2dUvAnimationComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spriteId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (animationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentTimeNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentFrame);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lastSyncNormalTimeNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (tickTime);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loop);
        EMERGENCE_MAPPING_REGISTER_REGULAR (flipU);
        EMERGENCE_MAPPING_REGISTER_REGULAR (flipV);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTER_REGULAR (waitingForAnimationToLoad);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
