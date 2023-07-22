#include <Celerity/Render/Foundation/Viewport.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Viewport::Reflection &Viewport::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Viewport);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (targetFrameBuffer);
        EMERGENCE_MAPPING_REGISTER_REGULAR (sortIndex);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);
        EMERGENCE_MAPPING_REGISTER_REGULAR (clearColor);
        EMERGENCE_MAPPING_REGISTER_REGULAR (sortMode);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
