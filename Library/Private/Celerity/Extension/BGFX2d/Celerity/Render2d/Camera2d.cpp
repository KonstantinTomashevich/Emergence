#include <Celerity/Render2d/Camera2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Camera2d::Reflection &Camera2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Camera2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
