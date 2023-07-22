#include <Celerity/Assembly/PrototypeAssemblyComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const PrototypeAssemblyComponent::Reflection &PrototypeAssemblyComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PrototypeAssemblyComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedAssemblyState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalAssemblyState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedCurrentComponentIndex);
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalCurrentComponentIndex);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
