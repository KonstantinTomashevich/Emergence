#include <Celerity/Render2d/Sprite2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2d::Reflection &Sprite2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spriteId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceResourceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uv);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
