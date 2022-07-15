#pragma once

#include <Celerity/Standard/UniqueId.hpp>

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

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId descriptorId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
