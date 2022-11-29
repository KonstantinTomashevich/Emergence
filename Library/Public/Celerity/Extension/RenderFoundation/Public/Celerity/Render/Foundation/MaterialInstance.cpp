#include <Celerity/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const MaterialInstance::Reflection &MaterialInstance::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MaterialInstance);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UniformVector4fValue::Reflection &UniformVector4fValue::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformVector4fValue);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniformName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (value);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UniformMatrix3x3fValue::Reflection &UniformMatrix3x3fValue::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformMatrix3x3fValue);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniformName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (value);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UniformMatrix4x4fValue::Reflection &UniformMatrix4x4fValue::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformMatrix4x4fValue);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniformName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (value);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UniformSamplerValue::Reflection &UniformSamplerValue::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformSamplerValue);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniformName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (textureId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
