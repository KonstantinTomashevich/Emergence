#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Render/Foundation/Material.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains all the information about a uniform.
struct CelerityRenderFoundationModelApi UniformDescription final
{
    /// \brief Name of the uniform.
    Memory::UniqueString name;

    /// \brief Type of the uniform.
    Render::Backend::UniformType type = Render::Backend::UniformType::VECTOR_4F;

    union
    {
        /// \brief Stage for uniforms of Render::Backend::UniformType::SAMPLER type.
        std::uint8_t textureStage = 0u;
    };

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Content of material file that stores all the information about this material
struct CelerityRenderFoundationModelApi MaterialAsset final
{
    /// \brief Id of a fragment shader used by this material.
    Memory::UniqueString vertexShader;

    /// \brief Id of a vertex shader used by this material.
    Memory::UniqueString fragmentShader;

    /// \brief Lists all the uniforms of this material.
    Container::Vector<UniformDescription> uniforms {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"MaterialUniforms"}}};

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::FieldId uniforms;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
