#include <Celerity/Input/InputActionHolder.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const InputActionHolder::Reflection &InputActionHolder::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputActionHolder);
        EMERGENCE_MAPPING_REGISTER_REGULAR (action);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dispatchType);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (dispatchType, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedDispatchDone);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
