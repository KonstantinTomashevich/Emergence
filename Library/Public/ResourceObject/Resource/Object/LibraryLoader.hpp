#pragma once

#include <Container/String.hpp>

#include <Resource/Object/Library.hpp>

#include <ResourceProvider/ResourceProvider.hpp>

namespace Emergence::Resource::Object
{
/// \brief Encapsulates request for specific object to be loaded.
struct LibraryLoadingTask final
{
    /// \brief Id of object to be loaded.
    Memory::UniqueString objectId;
};

/// \brief Loads libraries from sequences of LibraryLoadingTask.
class LibraryLoader final
{
public:
    /// \brief Constructs loader with given type manifest.
    LibraryLoader (ResourceProvider::ResourceProvider *_resourceProvider, TypeManifest _typeManifest) noexcept;

    LibraryLoader (const LibraryLoader &_other) = delete;

    LibraryLoader (LibraryLoader &&_other) = delete;

    ~LibraryLoader () noexcept = default;

    Library Load (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept;

    LibraryLoader &operator= (const LibraryLoader &_other) = delete;

    LibraryLoader &operator= (LibraryLoader &&_other) = delete;

private:
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Loads content of given object. Recursively loads content of all parents of this object.
    bool LoadObject (Memory::UniqueString _objectId, bool _loadingAsParent) noexcept;

    /// \brief Fills ::objectList for current loading routine. Loads object data as well.
    void FormObjectList (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept;

    /// \brief Applies post processing to object at given index: inheritance and injection features.
    void PostProcessObject (std::size_t _indexInList) noexcept;

    ResourceProvider::ResourceProvider *resourceProvider;
    Library currentLibrary;

    /// \brief Lists all objects to be loaded in current loading routine.
    Container::Vector<Memory::UniqueString> objectList {GetAllocationGroup ()};

    /// \brief Contains index in ::objectList for every object to be loaded.
    /// \details Needed for internal recursive inheritance check.
    Container::HashMap<Memory::UniqueString, std::size_t> indexInObjectList {GetAllocationGroup ()};

    TypeManifest typeManifest;
};
} // namespace Emergence::Resource::Object

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Resource::Object::LibraryLoadingTask)
