#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2d;

struct Material2dInstance final
{
    Memory::UniqueString resourceId;
    Memory::UniqueString materialResourceId;

    Container::Vector<uint8_t> uniforms {Memory::Profiler::AllocationGroup::Top ()};

    void InitForMaterial (const Material2d *_material) noexcept;

    void SetUniformById (const Material2d *_material, Memory::UniqueString _id, const void *_source) noexcept;

    void SetUniformByIndex (const Material2d *_material, std::size_t _index, const void *_source) noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId resourceId;
        StandardLayout::FieldId materialResourceId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
