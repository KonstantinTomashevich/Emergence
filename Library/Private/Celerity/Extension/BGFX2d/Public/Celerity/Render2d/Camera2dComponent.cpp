#include <Celerity/Render2d/Camera2dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Camera2dComponent::Reflection &Camera2dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Camera2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (orthographicSize);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
