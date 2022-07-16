#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Patch.hpp>

namespace Emergence::Celerity
{
/// \brief Contains info about components that will be attached to
///        object or spawned as sub-objects inside assembly routine.
struct AssemblyDescriptor final
{
    /// \brief Unique id of this descriptor.
    Memory::UniqueString id;

    /// \brief Component data represented as patches that are applied to freshly constructed object.
    Container::Vector<StandardLayout::Patch> components {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
