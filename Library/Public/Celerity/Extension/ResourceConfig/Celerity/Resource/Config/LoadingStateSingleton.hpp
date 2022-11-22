#pragma once

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains public state of resource config loading routine.
/// \invariant Read-only for users: external changes are not expected by routine code.
struct ResourceConfigLoadingStateSingleton final
{
    /// \brief Contains state and meta of one particular config type.
    struct TypeState final
    {
        /// \brief Mapping of associated config type.
        StandardLayout::Mapping type;

        /// \brief Id of a field that contains config name.
        StandardLayout::FieldId nameField;

        /// \brief Folder that contains config resources.
        Container::String folder;

        /// \brief Whether all configs of this type are loaded.
        bool loaded = false;
    };

    /// \brief Whether config path mapping is loaded and ready to use.
    bool pathMappingLoaded = false;

    /// \brief Contains state for each registered config type.
    Container::Vector<TypeState> typeStates {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId pathMappingLoaded;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
