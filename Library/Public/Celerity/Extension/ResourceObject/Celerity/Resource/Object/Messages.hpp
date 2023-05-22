#pragma once

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Request resource objects to be loaded.
struct ResourceObjectRequest final
{
    /// \brief Ids of resource objects to be loaded.
    Container::Vector<Memory::UniqueString> objects {Memory::Profiler::AllocationGroup::Top()};

    /// \brief Should objects be reloaded if it is already loaded.
    bool forceReload = false;

    struct Reflection final
    {
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that object requested through ResourceObjectRequest is loaded.
struct ResourceObjectLoadedResponse final
{
    /// \brief Object that was loaded.
    Memory::UniqueString objectId;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
