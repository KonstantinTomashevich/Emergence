#pragma once

#include <CelerityResourceConfigApi.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Requests all configs of selected type to be loaded into the world.
struct CelerityResourceConfigApi ResourceConfigRequest final
{
    /// \brief Selected config type.
    StandardLayout::Mapping type;

    /// \brief If true, configs will be reloaded from the disk even if they are already loaded into the world.
    bool forceReload = false;

    struct CelerityResourceConfigApi Reflection final
    {
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that all configs of given type are loaded into the world.
struct CelerityResourceConfigApi ResourceConfigLoadedResponse final
{
    /// \brief Type of configs that were loaded.
    StandardLayout::Mapping type;

    struct CelerityResourceConfigApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity