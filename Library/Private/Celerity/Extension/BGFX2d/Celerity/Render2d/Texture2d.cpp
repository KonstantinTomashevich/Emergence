#include <Celerity/Render2d/Texture2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Texture2d::Reflection &Texture2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Texture2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (resourceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
