#include <Celerity/Render2d/Material2d.hpp>

#include <Math/Matrix3x3f.hpp>
#include <Math/Matrix4x4f.hpp>
#include <Math/Vector4f.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
std::size_t GetUniformSize (Uniform2dType _type) noexcept
{
    switch (_type)
    {
    case Uniform2dType::VECTOR_4F:
        return sizeof (Math::Vector4f);

    case Uniform2dType::MATRIX_3X3F:
        return sizeof (Math::Matrix3x3f);

    case Uniform2dType::MATRIX_4X4F:
        return sizeof (Math::Matrix4x4f);

    case Uniform2dType::SAMPLER:
        return sizeof (Memory::UniqueString);
    }

    assert (false);
    return 0u;
}

const Uniform2d::Reflection &Uniform2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Uniform2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);
        EMERGENCE_MAPPING_REGISTER_REGULAR (count);

        EMERGENCE_MAPPING_REGISTER_REGULAR (offset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Material2d::Reflection &Material2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Material2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (resourceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vertexShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fragmentShader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
