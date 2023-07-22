#pragma once

#include <CelerityResourceObjectModelApi.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Request resource objects to be loaded.
struct CelerityResourceObjectModelApi ResourceObjectRequest final
{
    /// \brief Id of resource object to be loaded.
    Memory::UniqueString objectId;

    /// \brief Should objects be reloaded if it is already loaded.
    bool forceReload = false;

    struct CelerityResourceObjectModelApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that object requested through ResourceObjectRequest is loaded.
struct CelerityResourceObjectModelApi ResourceObjectLoadedResponse final
{
    /// \brief Object that was loaded.
    Memory::UniqueString objectId;

    struct CelerityResourceObjectModelApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
