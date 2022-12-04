#include <Celerity/Input/InputTriggers.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const KeyTrigger::Reflection &KeyTrigger::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (KeyTrigger);
        EMERGENCE_MAPPING_REGISTER_REGULAR (actionToSend);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerCode);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerTargetState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerType);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dispatchType);
        EMERGENCE_MAPPING_REGISTER_REGULAR (currentKeyState);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
