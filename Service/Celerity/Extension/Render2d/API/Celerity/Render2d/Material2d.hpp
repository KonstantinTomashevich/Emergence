#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents successfully loaded material asset.
/// \details Materials are used to bind shader programs and define uniforms supported by these shader programs.
///          That means that materials define how object is rendered and provide customizable parameters as
///          uniforms. Parameter customization is done through Material2dInstance.
struct Material2d final
{
    Material2d () noexcept = default;

    Material2d (const Material2d &_other) = delete;

    Material2d (Material2d &&_other) = delete;

    ~Material2d () noexcept;

    Material2d &operator= (const Material2d &_other) = delete;

    Material2d &operator= (Material2d &&_other) = delete;

    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Name of the vertex shader that is used by this material.
    /// \invariant Cannot be changed.
    Memory::UniqueString vertexShader;

    /// \brief Name of the fragment shader that is used by this material.
    /// \invariant Cannot be changed.
    Memory::UniqueString fragmentShader;

    /// \brief Implementation-specific native handle.
    uintptr_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Defines supported uniform types.
enum class Uniform2dType : uint8_t
{
    /// \brief 4 dimensional vector.
    VECTOR_4F = 0u,

    /// \brief 3x3 sized matrix.
    MATRIX_3X3F,

    /// \brief 4x4 sized matrix.
    MATRIX_4X4F,

    /// \brief 2d texture sampler.
    SAMPLER,
};

/// \brief Represents parameter supported by Material2d. Part of Material2d asset.
struct Uniform2d final
{
    Uniform2d () noexcept = default;

    Uniform2d (const Uniform2d &_other) = delete;

    Uniform2d (Uniform2d &&_other) = delete;

    ~Uniform2d () noexcept;

    Uniform2d &operator= (const Uniform2d &_other) = delete;

    Uniform2d &operator= (Uniform2d &&_other) = delete;

    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Unique name of this uniform.
    /// \invariant Cannot be changed.
    Memory::UniqueString name;

    /// \brief Value type that can be stored in this uniform.
    /// \invariant Cannot be changed.
    Uniform2dType type = Uniform2dType::VECTOR_4F;

    /// \brief Implementation-specific native handle.
    uintptr_t nativeHandle = 0u;

    union
    {
        /// \brief Stage to which texture of Uniform2dType::SAMPLER will be passed.
        /// \details Allowed to be changed during runtime.
        uint8_t textureStage = 0u;
    };

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::FieldId textureStage;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
