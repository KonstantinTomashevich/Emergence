#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Request particular object from particular folder to be loaded.
struct AssetObjectRequest final
{
    /// \brief Folder from which object will be loaded.
    Container::String folder;

    /// \brief Name of object to be loaded.
    Memory::UniqueString object;

    /// \brief Should object be reloaded if it is already loaded.
    bool forceReload = false;

    struct Reflection final
    {
        StandardLayout::FieldId object;
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Request all objects from particular folder to be loaded.
struct AssetObjectFolderRequest final
{
    /// \brief Folder from which objects will be loaded.
    Container::String folder;

    /// \brief Should folder be reloaded if it is already loaded.
    bool forceReload = false;

    struct Reflection final
    {
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that object requested through AssetObjectRequest is loaded.
struct AssetObjectLoadedResponse final
{
    /// \brief Folder from which object was loaded.
    Container::String folder;

    /// \brief Name of the object that was loaded.
    Memory::UniqueString object;

    struct Reflection final
    {
        StandardLayout::FieldId object;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Informs that folder requested through AssetObjectFolderRequest is loaded.
struct AssetObjectFolderLoadedResponse final
{
    /// \brief Folder all objects from which were loaded.
    Container::String folder;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
