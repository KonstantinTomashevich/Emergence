#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Requests all configs of selected type to be loaded into the world.
struct AssetConfigRequest final
{
    /// \brief Selected config type.
    StandardLayout::Mapping type;

    /// \brief If true, configs will be reloaded from the disk even if they are already loaded into the world.
    bool forceReload = false;

    struct Reflection final
    {
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that all configs of given type are loaded into the world.
struct AssetConfigLoadedResponse final
{
    /// \brief Type of configs that were loaded.
    StandardLayout::Mapping type;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
