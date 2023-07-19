#pragma once

#include <CelerityRenderFoundationApi.hpp>

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <Render/Backend/Program.hpp>
#include <Render/Backend/Uniform.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents successfully loaded material asset.
/// \details Materials are used to bind shader programs and define uniforms supported by these shader programs.
///          That means that materials define how object is rendered and provide customizable parameters as
///          uniforms. Parameter customization is done through MaterialInstance.
struct CelerityRenderFoundationApi Material final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the vertex shader that is used by this material.
    /// \invariant Cannot be changed.
    Memory::UniqueString vertexShader;

    /// \brief Name of the fragment shader that is used by this material.
    /// \invariant Cannot be changed.
    Memory::UniqueString fragmentShader;

    /// \brief Underlying implementation object.
    Render::Backend::Program program;

    struct CelerityRenderFoundationApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Represents parameter supported by Material. Part of Material asset.
struct CelerityRenderFoundationApi Uniform final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Unique name of this uniform.
    /// \invariant Cannot be changed.
    Memory::UniqueString name;

    /// \brief Value type that can be stored in this uniform.
    /// \invariant Cannot be changed.
    Render::Backend::UniformType type = Render::Backend::UniformType::VECTOR_4F;

    /// \brief Underlying implementation object.
    /// \details Field is mutable, because render backend objects are technically handles and by modifying them
    ///          we work with underlying implementation that operates under different read-write ruleset.
    mutable Render::Backend::Uniform uniform;

    union
    {
        /// \brief Stage to which texture of Render::Backend::UniformType::SAMPLER will be passed.
        /// \details Allowed to be changed during runtime.
        std::uint8_t textureStage = 0u;
    };

    struct CelerityRenderFoundationApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
