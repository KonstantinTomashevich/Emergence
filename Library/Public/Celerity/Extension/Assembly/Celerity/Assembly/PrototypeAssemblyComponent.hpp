#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates possible state of assembly process in associated pipeline (normal or fixed).
enum class AssemblyState : std::uint8_t
{
    /// \brief Blocked, because assembly in other pipeline is not finished yet.
    /// \details In order to start normal assembly, we need to finish fixed assembly first.
    BLOCKED,

    /// \brief Components of associated pipeline must be assembled right away during next assembly execution.
    IN_NEED_OF_IMMEDIATE_ASSEMBLY,

    /// \brief Components of associated pipeline are ready to be instanced, but instancing can be delayed.
    WAITING_FOR_ASSEMBLY,

    /// \brief Assembly routine is finished in this pipeline.
    ASSEMBLED,
};

/// \brief Attached to PrototypeComponent during assembly routine in
///        order to store transient data used to assemble the prototype.
/// \details Normally, the only thing user needs to know about this component is that if there are no components of
///          this type, then all assembly requests were addressed. It is useful to check whether loading is done.
struct PrototypeAssemblyComponent final
{
    /// \brief Id of the object to which this component is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Current state of assembly routine in fixed pipeline.
    AssemblyState fixedAssemblyState = AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY;

    /// \brief Current state of assembly routine in normal pipeline.
    AssemblyState normalAssemblyState = AssemblyState::BLOCKED;

    /// \brief Index of component from which next instancing batch in fixed pipeline will start.
    std::size_t fixedCurrentComponentIndex = 0u;

    /// \brief Index of component from which next instancing batch in normal pipeline will start.
    std::size_t normalCurrentComponentIndex = 0u;

    // TODO: Use flat hash map.

    /// \brief Used to save id replacement map while assembly is in intermediate state
    ///        (not all components are assembled yet).
    /// \details To correctly resolve references between fixed-assembled and normal-assembled types, or even between
    ///          components that were assembled in one pipeline, but in different frames, we need to save id replacement
    ///          map and reuse it during next passes.
    Container::Vector<Container::HashMap<UniqueId, UniqueId>> intermediateIdReplacement {
        Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId fixedAssemblyState;
        StandardLayout::FieldId normalAssemblyState;
        StandardLayout::FieldId fixedCurrentComponentIndex;
        StandardLayout::FieldId normalCurrentComponentIndex;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
