#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Render/Foundation/Material.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains all the information about a uniform.
struct UniformDescription final
{
    /// \brief Name of the uniform.
    Memory::UniqueString name;

    /// \brief Type of the uniform.
    Render::Backend::UniformType type = Render::Backend::UniformType::VECTOR_4F;

    union
    {
        /// \brief Stage for uniforms of Render::Backend::UniformType::SAMPLER type.
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

/// \brief Content of material file that stores all the information about this material
/// \invariant Path to material header file is <material root folder>/<material id>.material.<format extension>,
///            where material root folder is any registered root folder for materials, material id is any string
///            that may include '/' for folder grouping, and format extension is either yaml or bin.
struct MaterialAsset final
{
    /// \brief Id of a fragment shader used by this material.
    /// \details Shaders reside in shader root folders and their ids are allowed to contain '/',
    ///          but their extension is implementation specific.
    Memory::UniqueString vertexShader;

    /// \brief Id of a vertex shader used by this material.
    /// \details Shaders reside in shader root folders and their ids are allowed to contain '/',
    ///          but their extension is implementation specific.
    Memory::UniqueString fragmentShader;

    /// \brief Lists all the uniforms of this material.
    Container::Vector<UniformDescription> uniforms {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"MaterialUniforms"}}};

    struct Reflection final
    {
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::FieldId uniforms;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
