#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Vector4f.hpp>
#include <Math/Matrix3x3f.hpp>
#include <Math/Matrix4x4f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2d;

struct Material2dInstance final
{
    Memory::UniqueString assetId;
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId materialId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UniformVector4fValue final
{
    Memory::UniqueString assetId;
    Memory::UniqueString uniformName;
    Math::Vector4f value = Math::Vector4f::ZERO;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId uniformName;
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UniformMatrix3x3fValue final
{
    Memory::UniqueString assetId;
    Memory::UniqueString uniformName;
    Math::Matrix3x3f value = Math::Matrix3x3f::ZERO;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId uniformName;
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UniformMatrix4x4fValue final
{
    Memory::UniqueString assetId;
    Memory::UniqueString uniformName;
    Math::Matrix4x4f value = Math::Matrix4x4f::ZERO;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId uniformName;
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UniformSamplerValue final
{
    Memory::UniqueString assetId;
    Memory::UniqueString uniformName;
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    Memory::UniqueString textureId;
    uint8_t textureStage = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId uniformName;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId textureId;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
