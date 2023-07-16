#include <Celerity/Physics2d/TriggerContact2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const TriggerContact2d::Reflection &TriggerContact2d::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TriggerContact2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerContactId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (intruderObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (triggerShapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (intruderShapeId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
