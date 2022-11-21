#pragma once

#include <Container/HashSet.hpp>
#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Public information about asset object loading.
struct ResourceObjectLoadingStateSingleton final
{
    /// \brief Set of directories that are considered to be loaded.
    Container::HashSet<Container::String> loadedFolders {Memory::Profiler::AllocationGroup {}};

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
