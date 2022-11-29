#pragma once

#include <Celerity/Render2d/Material.hpp>
#include <Celerity/Render2d/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/MuteWarnings.hpp>

namespace Emergence::Celerity
{
/// \brief Content of material instance header file that stores main information about material instance.
/// \invariant Path to material instance header file is
///            <material instance root folder>/<material instance id>.material.instance.<format extension>,
///            where material instance root folder is any registered root folder for material instances,
///            material instance id is any string that may include '/' for folder grouping,
///            and format extension is either yaml or bin.
struct MaterialInstanceAssetHeader final
{
    /// \brief Parent material instances id if parent exists for this instance.
    /// \details Material instance inheritance feature allows material instances to inherit and override values
    ///          from parent material instance. It allows to reduce duplication and reuse common uniform configuration.
    Memory::UniqueString parent;

    /// \brief Id of material to which this instance is connected.
    Memory::UniqueString material;

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::FieldId material;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Item of uniform values bundle file that contains values for material uniforms.
/// \invariant Path to uniform values bundle file is
///            <material instance root folder>/<material instance id>.uniform.values.<format extension>,
///            where material instance root folder is any registered root folder for material instances,
///            material instance id is any string that may include '/' for folder grouping,
///            and format extension is either yaml or bin.
struct UniformValueBundleItem final
{
    UniformValueBundleItem () noexcept = default;

    UniformValueBundleItem (Memory::UniqueString _name, const Math::Vector4f &_value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix3x3f _value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name, Math::Matrix4x4f _value) noexcept;

    UniformValueBundleItem (Memory::UniqueString _name, const Memory::UniqueString &_textureId) noexcept;

    /// \brief Name of the uniform to which value will be assigned.
    Memory::UniqueString name;

    /// \brief Value type used for deserialization and correctness check.
    Render::Backend::UniformType type = Render::Backend::UniformType::VECTOR_4F;

    struct
    {
        /// \brief Field for Render::Backend::UniformType::VECTOR_4F values.
        Math::Vector4f vector4f {Math::NoInitializationFlag::Confirm ()};

        /// \brief Field for Render::Backend::UniformType::MATRIX_3X3F values.
        Math::Matrix3x3f matrix3x3f {Math::NoInitializationFlag::Confirm ()};

        // This structure is only used during material deserialization,
        // therefore we shouldn't worry about small waste of padding here.
        BEGIN_IGNORING_PADDING_WARNING
        /// \brief Field for Render::Backend::UniformType::MATRIX_4X4F values.
        Math::Matrix4x4f matrix4x4f {Math::NoInitializationFlag::Confirm ()};
        END_IGNORING_PADDING_WARNING

        /// \brief Id of a texture for Render::Backend::UniformType::SAMPLER values.
        Memory::UniqueString textureId;
    };

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId vector4f;
        StandardLayout::FieldId matrix3x3f;
        StandardLayout::FieldId matrix4x4f;
        StandardLayout::FieldId textureId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
