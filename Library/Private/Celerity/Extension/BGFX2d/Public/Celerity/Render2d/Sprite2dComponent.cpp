#include <Celerity/Render2d/Sprite2dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2dComponent::Reflection &Sprite2dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spriteId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uv);
        EMERGENCE_MAPPING_REGISTER_REGULAR (halfSize);
        EMERGENCE_MAPPING_REGISTER_REGULAR (layer);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
