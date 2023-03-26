#include <Celerity/Physics2d/Physics2dDebugDrawSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Physics2dDebugDrawSingleton::Reflection &Physics2dDebugDrawSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Physics2dDebugDrawSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled);
        EMERGENCE_MAPPING_REGISTER_REGULAR (wasEnabled);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
