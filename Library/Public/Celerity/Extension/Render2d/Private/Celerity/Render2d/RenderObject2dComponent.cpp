#include <Celerity/Render2d/RenderObject2dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const RenderObject2dComponent::Reflection &RenderObject2dComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RenderObject2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (global);
        EMERGENCE_MAPPING_REGISTER_REGULAR (local);
        EMERGENCE_MAPPING_REGISTER_REGULAR (globalDirty);
        EMERGENCE_MAPPING_REGISTER_REGULAR (localDirty);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const LocalBounds2dComponent::Reflection &LocalBounds2dComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LocalBounds2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (renderObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (bounds);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dirty);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
