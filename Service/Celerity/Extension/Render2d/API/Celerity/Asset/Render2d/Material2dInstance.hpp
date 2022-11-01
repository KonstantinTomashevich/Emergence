#pragma once

#include <Celerity/Render2d/Material2d.hpp>
#include <Celerity/Render2d/Material2dInstance.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2dInstanceAssetHeader final
{
    Memory::UniqueString parent;
    Memory::UniqueString material;

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::FieldId material;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UniformValueBundleItem final
{
    UniformValueBundleItem () noexcept = default;

    UniformValueBundleItem (Memory::UniqueString _name, const Math::Vector4f &_value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix3x3f _value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix4x4f _value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name,
                            const Memory::UniqueString &_textureId,
                            uint8_t _textureStage) noexcept;

    Memory::UniqueString name;
    Uniform2dType type = Uniform2dType::VECTOR_4F;

    struct
    {
        Math::Vector4f vector4f {Math::NoInitializationFlag::Confirm ()};
        Math::Matrix3x3f matrix3x3f {Math::NoInitializationFlag::Confirm ()};
        Math::Matrix4x4f matrix4x4f {Math::NoInitializationFlag::Confirm ()};

        struct
        {
            Memory::UniqueString textureId;
            uint8_t textureStage;
        };
    };

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId vector4f;
        StandardLayout::FieldId matrix3x3f;
        StandardLayout::FieldId matrix4x4f;
        StandardLayout::FieldId textureId;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Material2dInstanceLoadingState final
{
    Memory::UniqueString assetId;
    UniqueId assetUserId;
    Memory::UniqueString parentId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId parentId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
