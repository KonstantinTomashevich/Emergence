#include <Platformer/Layer/LayerSetupComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LayerSetupComponent::Reflection &LayerSetupComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LayerSetupComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (layer);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
