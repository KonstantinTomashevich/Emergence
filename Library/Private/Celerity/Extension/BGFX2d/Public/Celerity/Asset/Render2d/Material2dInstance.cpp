#include <Celerity/Asset/Render2d/Material2dInstance.hpp>

#include <StandardLayout/MappingRegistration.hpp>
#include <utility>

namespace Emergence::Celerity
{
const Material2dInstanceAssetHeader::Reflection &Material2dInstanceAssetHeader::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Material2dInstanceAssetHeader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (material);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

UniformValueBundleItem::UniformValueBundleItem (Memory::UniqueString _name, const Math::Vector4f &_value) noexcept
    : name (_name),
      vector4f (_value)
{
}

UniformValueBundleItem::UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix3x3f _value) noexcept
    : name (_name),
      type (Uniform2dType::MATRIX_3X3F),
      matrix3x3f (std::move (_value))
{
}

UniformValueBundleItem::UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix4x4f _value) noexcept
    : name (_name),
      type (Uniform2dType::MATRIX_4X4F),
      matrix4x4f (std::move (_value))
{
}

UniformValueBundleItem::UniformValueBundleItem (Memory::UniqueString _name,
                                                const Memory::UniqueString &_textureId,
                                                uint8_t _textureStage) noexcept
    : name (_name),
      type (Uniform2dType::SAMPLER),
      textureId (_textureId),
      textureStage (_textureStage)
{
}

const UniformValueBundleItem::Reflection &UniformValueBundleItem::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UniformValueBundleItem);
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
        EMERGENCE_MAPPING_REGISTER_REGULAR (textureStage);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Material2dInstanceLoadingState::Reflection &Material2dInstanceLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Material2dInstanceLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parentId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
