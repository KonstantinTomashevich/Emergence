#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enables assembly routine for this object by connecting it with assembly descriptor.
struct PrototypeComponent final
{
    /// \brief Id of the object to which this component is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Id of AssemblyDescriptor using which object will be assembled.
    Memory::UniqueString descriptorId;

    // TODO: Use flat hash map.

    /// \brief Used to save id replacement map while assembly is in intermediate state
    ///        (assembled in normal, but not in fixed and vice versa).
    /// \details Each prototype is technically assembled two times: once in fixed update and once in normal update.
    ///          To correctly resolve references between normal-assembled and fixed-assembled types we need to save
    ///          id replacement map during first pass (doesn't matter whether it is normal or fixed) and reuse it
    ///          during second pass.
    Container::Vector<Container::HashMap<UniqueId, UniqueId>> intermediateIdReplacement {
        Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId descriptorId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
