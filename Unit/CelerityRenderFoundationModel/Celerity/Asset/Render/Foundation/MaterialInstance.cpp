#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
// NOLINTNEXTLINE(modernize-use-equals-default): We cannot use default for no-initialization due to union.
UniformValueDescription::UniformValueDescription () noexcept
{
}

UniformValueDescription::UniformValueDescription (const UniformValueDescription &_other) noexcept
    : name (_other.name),
      type (_other.type),
      matrix4x4f (_other.matrix4x4f)
{
}

UniformValueDescription::UniformValueDescription (UniformValueDescription &&_other) noexcept
    : name (_other.name),
      type (_other.type),
      matrix4x4f (_other.matrix4x4f)
{
}

UniformValueDescription::UniformValueDescription (Memory::UniqueString _name, const Math::Vector4f &_value) noexcept
    : name (_name),
      vector4f (_value)
{
}

UniformValueDescription::UniformValueDescription (Memory::UniqueString _name, Math::Matrix3x3f _value) noexcept
    : name (_name),
      type (Render::Backend::UniformType::MATRIX_3X3F),
      matrix3x3f (std::move (_value))
{
}

UniformValueDescription::UniformValueDescription (Memory::UniqueString _name, Math::Matrix4x4f _value) noexcept
    : name (_name),
      type (Render::Backend::UniformType::MATRIX_4X4F),
      matrix4x4f (std::move (_value))
{
}

UniformValueDescription::UniformValueDescription (Memory::UniqueString _name,
                                                  const Memory::UniqueString &_textureId) noexcept
    : name (_name),
      type (Render::Backend::UniformType::SAMPLER),
      textureId (_textureId)
{
}

// NOLINTNEXTLINE(modernize-use-equals-default): We cannot use default here due to union.
UniformValueDescription::~UniformValueDescription () noexcept
{
}

UniformValueDescription &UniformValueDescription::operator= (const UniformValueDescription &_other) noexcept
{
    if (this != &_other)
    {
        this->~UniformValueDescription ();
        new (this) UniformValueDescription (_other);
    }

    return *this;
}

UniformValueDescription &UniformValueDescription::operator= (UniformValueDescription &&_other) noexcept
{
    if (this != &_other)
    {
        this->~UniformValueDescription ();
        new (this) UniformValueDescription (std::move (_other));
    }

    return *this;
}

const UniformValueDescription::Reflection &UniformValueDescription::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformValueDescription);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vector4f);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (matrix3x3f);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (matrix4x4f);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 3u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (textureId);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MaterialInstanceAsset::Reflection &MaterialInstanceAsset::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MaterialInstanceAsset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (material);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniforms);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
