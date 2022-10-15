#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class Uniform2dType
{
    VECTOR_4F = 0u,
    MATRIX_3X3F,
    MATRIX_4X4F,
    SAMPLER,
};

std::size_t GetUniformSize (Uniform2dType _type) noexcept;

struct Uniform2d
{
    Memory::UniqueString id;
    Uniform2dType type = Uniform2dType::VECTOR_4F;
    std::uint64_t count = 0u;

    std::uint64_t offset = 0u;
    std::uintptr_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId type;
        StandardLayout::FieldId count;

        StandardLayout::FieldId offset;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Material2d final
{
    Memory::UniqueString resourceId;
    Memory::UniqueString vertexShader;
    Memory::UniqueString fragmentShader;
    std::uintptr_t nativeHandle = 0u;

    Container::Vector<Uniform2d> uniforms {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId resourceId;
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
