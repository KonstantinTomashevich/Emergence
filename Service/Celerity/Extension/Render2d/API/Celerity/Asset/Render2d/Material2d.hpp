#pragma once

#include <Celerity/Render2d/Material2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Content of material header file that stores main information about material.
/// \invariant Path to material header file is <material root folder>/<material id>.material.<format extension>,
///            where material root folder is any registered root folder for materials, material id is any string
///            that may include '/' for folder grouping, and format extension is either yaml or bin.
struct Material2dAssetHeader final
{
    /// \brief Id of a fragment shader used by this material.
    /// \details Shaders reside in shader root folders and their ids are allowed to contain '/',
    ///          but their extension is implementation specific.
    Memory::UniqueString vertexShader;

    /// \brief Id of a vertex shader used by this material.
    /// \details Shaders reside in shader root folders and their ids are allowed to contain '/',
    ///          but their extension is implementation specific.
    Memory::UniqueString fragmentShader;

    struct Reflection final
    {
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Item of uniforms bundle file that contains all supported uniforms for the material.
/// \invariant Path to uniforms bundle file is <material root folder>/<material id>.uniforms.<format extension>,
///            where material root folder is any registered root folder for materials, material id is any string
///            that may include '/' for folder grouping, and format extension is either yaml or bin.
struct Uniform2dBundleItem final
{
    /// \brief Name of the uniform.
    Memory::UniqueString name;

    /// \brief Type of the uniform.
    Uniform2dType type = Uniform2dType::VECTOR_4F;

    union
    {
        /// \brief Stage for uniforms of Uniform2dType::SAMPLER type.
        uint8_t textureStage = 0u;
    };

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
