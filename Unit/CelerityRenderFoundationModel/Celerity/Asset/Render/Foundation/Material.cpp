#include <Celerity/Asset/Render/Foundation/Material.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const UniformDescription::Reflection &UniformDescription::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformDescription);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (textureStage);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MaterialAsset::Reflection &MaterialAsset::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MaterialAsset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vertexShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fragmentShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniforms);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
