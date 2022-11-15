#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2d final
{
    Material2d () noexcept = default;

    Material2d (const Material2d &_other) = delete;

    Material2d (Material2d &&_other) = delete;

    ~Material2d () noexcept;

    Material2d &operator= (const Material2d &_other) = delete;

    Material2d &operator= (Material2d &&_other) = delete;

    Memory::UniqueString assetId;
    Memory::UniqueString vertexShader;
    Memory::UniqueString fragmentShader;
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

enum class Uniform2dType : uint8_t
{
    VECTOR_4F = 0u,
    MATRIX_3X3F,
    MATRIX_4X4F,
    SAMPLER,
};

struct Uniform2d final
{
    Uniform2d () noexcept = default;

    Uniform2d (const Uniform2d &_other) = delete;

    Uniform2d (Uniform2d &&_other) = delete;

    ~Uniform2d () noexcept;

    Uniform2d &operator= (const Uniform2d &_other) = delete;

    Uniform2d &operator= (Uniform2d &&_other) = delete;

    Memory::UniqueString assetId;
    Memory::UniqueString name;
    Uniform2dType type = Uniform2dType::VECTOR_4F;
    uintptr_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
