#include <Celerity/Render2d/Material2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Material2d::Reflection &Material2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Material2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vertexShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fragmentShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Uniform2d::Reflection &Uniform2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Uniform2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);
        EMERGENCE_MAPPING_REGISTER_REGULAR (count);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
