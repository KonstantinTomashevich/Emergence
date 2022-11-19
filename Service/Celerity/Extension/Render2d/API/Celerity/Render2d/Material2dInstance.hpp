#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Matrix3x3f.hpp>
#include <Math/Matrix4x4f.hpp>
#include <Math/Vector4f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2d;

// TODO: Implement runtime material instance creation.

/// \brief Contains parameters for rendering with associated Material2d.
/// \details Material instances support inheritance on asset level, but it is flattened
///          during loading, therefore there is no data about inheritance in runtime objects.
struct Material2dInstance final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief As material instance references Material2d, it needs a unique asset user id.
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    /// \brief Id of the material, parameters to which this instance contains.
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

/// \brief Contains value for Uniform2dType::VECTOR_4F parameter. Part of Material2dInstance asset.
struct UniformVector4fValue final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the uniform that should receive this value.
    Memory::UniqueString uniformName;

    /// \brief Value to be sent.
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

/// \brief Contains value for Uniform2dType::MATRIX_3X3F parameter. Part of Material2dInstance asset.
struct UniformMatrix3x3fValue final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the uniform that should receive this value.
    Memory::UniqueString uniformName;

    /// \brief Value to be sent.
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

/// \brief Contains value for Uniform2dType::MATRIX_4X4F parameter. Part of Material2dInstance asset.
struct UniformMatrix4x4fValue final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the uniform that should receive this value.
    Memory::UniqueString uniformName;

    /// \brief Value to be sent.
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

/// \brief Contains value for Uniform2dType::SAMPLER parameter. Part of Material2dInstance asset.
struct UniformSamplerValue final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the uniform that should receive this value.
    Memory::UniqueString uniformName;

    /// \brief As uniform sampler value references Texture2d, it needs a unique asset user id.
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    /// \brief Id of the texture asset that should be sent as value.
    Memory::UniqueString textureId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId uniformName;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId textureId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
