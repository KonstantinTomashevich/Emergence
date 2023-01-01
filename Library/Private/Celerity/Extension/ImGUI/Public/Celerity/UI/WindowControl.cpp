#include <Celerity/UI/WindowControl.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const WindowControl::Reflection &WindowControl::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (WindowControl);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nodeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (viewportName);

        EMERGENCE_MAPPING_REGISTER_REGULAR (closable);
        EMERGENCE_MAPPING_REGISTER_REGULAR (minimizable);
        EMERGENCE_MAPPING_REGISTER_REGULAR (resizable);
        EMERGENCE_MAPPING_REGISTER_REGULAR (movable);
        EMERGENCE_MAPPING_REGISTER_REGULAR (hasTitleBar);
        EMERGENCE_MAPPING_REGISTER_REGULAR (pack);

        EMERGENCE_MAPPING_REGISTER_REGULAR (open);
        EMERGENCE_MAPPING_REGISTER_REGULAR (layout);

        EMERGENCE_MAPPING_REGISTER_REGULAR (anchor);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);

        EMERGENCE_MAPPING_REGISTER_REGULAR (onClosedAction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (onClosedActionDispatch);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
