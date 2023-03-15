#pragma once

#include <Container/String.hpp>

#include <Resource/Object/Library.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/FieldNameLookupCache.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Resource::Object
{
/// \brief Describes part of the library that needs to be loaded.
struct LibraryLoadingTask final
{
    /// \brief Relative path to the folder that contains objects.
    Container::String folder;

    /// \brief If not empty, only object with given name will be loaded from ::folder.
    /// \details This feature is designed for the editors, but can be used in other cases if the user needs it.
    Memory::UniqueString loadSelectedObject;
};

/// \brief Asynchronously loads libraries from sequences of LibraryLoadingTask.
class LibraryLoader final
{
public:
    /// \brief Name of the binary file in the folder that lists all dependencies of this folder.
    inline static const Container::String BINARY_FOLDER_DEPENDENCY_LIST = "ObjectFolderDependencies.bin";

    /// \brief Name of the YAML file in the folder that lists all dependencies of this folder.
    inline static const Container::String YAML_FOLDER_DEPENDENCY_LIST = "ObjectFolderDependencies.yaml";

    /// \brief Suffix for the object binary declaration file.
    inline static const Container::String BINARY_OBJECT_DECLARATION_SUFFIX = ".declaration.bin";

    /// \brief Suffix for the object YAML declaration file.
    inline static const Container::String YAML_OBJECT_DECLARATION_SUFFIX = ".declaration.yaml";

    /// \brief Suffix for the object binary body file.
    inline static const Container::String BINARY_OBJECT_BODY_SUFFIX = ".body.bin";

    /// \brief Suffix for the object YAML body file.
    inline static const Container::String YAML_OBJECT_BODY_SUFFIX = ".body.yaml";

    /// \brief Constructs loader with given type manifest.
    LibraryLoader (TypeManifest _typeManifest) noexcept;

    LibraryLoader (const LibraryLoader &_other) = delete;

    LibraryLoader (LibraryLoader &&_other) = delete;

    ~LibraryLoader () noexcept;

    /// \brief Starts asynchronously loading library from given sequence of tasks.
    /// \invariant Only one loading routine per loader instance is supported.
    void Begin (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept;

    /// \return Whether current loading routine is still going on.
    bool IsLoading () const noexcept;

    /// \return Loading tasks of current loading routine.
    /// \invariant Inside loading routine (between ::Begin and ::End calls).
    const Container::Vector<LibraryLoadingTask> &GetLoadingTasks () noexcept;

    /// \return Library created during last loading routine.
    /// \details Can only be called once per loading routine, because library is moved out of loader.
    /// \invariant `IsLoading` is `false`.
    Library End () noexcept;

    LibraryLoader &operator= (const LibraryLoader &_other) = delete;

    LibraryLoader &operator= (LibraryLoader &&_other) = delete;

private:
    struct ObjectListItem
    {
        Container::String folder;
        Memory::UniqueString name;
    };

    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Registers given folder and all its dependencies in ::folderList.
    void RegisterFolder (const Container::String &_folder) noexcept;

    /// \brief Fills ::folderList for current loading routine.
    void FormFolderList () noexcept;

    /// \brief Attempts to find object with given name in any of acknowledged folders
    ///        and calls ::LoadObjectDeclaration on found declaration if any.
    bool FindAndLoadDeclaration (Memory::UniqueString _objectName, bool _loadingAsParent) noexcept;

    /// \brief Loads declaration of given object. Recursively loads declarations of all parents of this object.
    void LoadObjectDeclaration (const Container::String &_objectFolder,
                                Memory::UniqueString _objectName,
                                bool _loadingAsParent) noexcept;

    /// \brief Fills ::objectList for current loading routine.
    void FormObjectList () noexcept;

    /// \brief Loads body of object that is stored in given cell of ::objectList.
    /// \details Object bodies are loaded sequentially one after another, not simultaneously.
    ///          This strategy is selected for several reasons:
    ///          - Persistent storage is expected to be main performance limiter here, not CPU,
    ///            therefore we would not gain a lot of speed by utilizing more CPU.
    ///          - Sequential loading allows to use caches to full extent, making loading much more efficient.
    void LoadObjectBody (std::size_t _indexInList) noexcept;

    std::atomic_flag loading;
    Library currentLibrary;
    Container::Vector<LibraryLoadingTask> loadingTasks {GetAllocationGroup ()};

    /// \brief Lists all folders that can provide objects for current loading routine.
    Container::Vector<Container::String> folderList {GetAllocationGroup ()};

    /// \brief Lists all objects to be loaded in current loading routine.
    Container::Vector<ObjectListItem> objectList {GetAllocationGroup ()};

    /// \brief Contains index in ::objectList for every object to be loaded.
    Container::HashMap<Memory::UniqueString, std::size_t> indexInObjectList {GetAllocationGroup ()};

    Serialization::Binary::PatchBundleDeserializer binaryPatchBundleDeserializer;
    Serialization::Yaml::PatchBundleDeserializer yamlPatchBundleDeserializer;

    TypeManifest typeManifest;
    Serialization::FieldNameLookupCache declarationFieldLookupCache {Declaration::Reflect ().mapping};
    uint64_t loadingStartTimeNs = 0u;
};

/// \brief Item of folder dependency list.
struct FolderDependency final
{
    static constexpr std::size_t RELATIVE_PATH_MAX_LENGTH = 128u;

    /// \brief Relative path from dependant folder to dependency folder.
    std::array<char, RELATIVE_PATH_MAX_LENGTH> relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Resource::Object

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Resource::Object::LibraryLoadingTask)
