#include <Celerity/UI/ButtonControl.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const ButtonControl::Reflection &ButtonControl::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ButtonControl);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nodeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (labelKey);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);

        EMERGENCE_MAPPING_REGISTER_REGULAR (onClickAction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (onClickActionDispatch);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
