#include <Celerity/Render2d/Viewport2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Viewport2d::Reflection &Viewport2d::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Viewport2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (cameraObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (sortIndex);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);
        EMERGENCE_MAPPING_REGISTER_REGULAR (clearColor);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
