#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Request particular object from particular folder to be loaded.
struct ResourceObjectRequest final
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
struct ResourceObjectFolderRequest final
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

/// \brief Informs that object requested through ResourceObjectRequest is loaded.
struct ResourceObjectLoadedResponse final
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

/// \brief Informs that folder requested through ResourceObjectFolderRequest is loaded.
struct ResourceObjectFolderLoadedResponse final
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
