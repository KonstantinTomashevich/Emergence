#include <Celerity/UI/CheckboxControl.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const CheckboxControl::Reflection &CheckboxControl::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CheckboxControl);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nodeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (labelKey);
        EMERGENCE_MAPPING_REGISTER_REGULAR (label);
        EMERGENCE_MAPPING_REGISTER_REGULAR (checked);

        EMERGENCE_MAPPING_REGISTER_REGULAR (onChangedAction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (onChangedActionDispatch);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
