#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Patch.hpp>

namespace Emergence::Celerity
{
struct AssemblyDescriptor final
{
    Memory::UniqueString id;
    Container::Vector<StandardLayout::Patch> objects {Memory::Profiler::AllocationGroup::Top()};

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
