#pragma once

#include <Asset/Object/Library.hpp>

#include <Container/String.hpp>

#include <Serialization/FieldNameLookupCache.hpp>
#include <Serialization/Yaml.hpp>
#include <Serialization/Binary.hpp>

namespace Emergence::Asset::Object
{
struct LibraryLoadingTask final
{
    Container::String folder;

    Memory::UniqueString loadSelectedObject;
};

class LibraryLoader final
{
public:
    inline static const Container::String YAML_FOLDER_DEPENDENCY_LIST = "ObjectFolderDependencies.yaml";

    inline static const Container::String BINARY_FOLDER_DEPENDENCY_LIST = "ObjectFolderDependencies.bin";

    inline static const Container::String BINARY_OBJECT_DECLARATION_SUFFIX = ".declaration.bin";

    inline static const Container::String YAML_OBJECT_DECLARATION_SUFFIX = ".declaration.yaml";

    inline static const Container::String BINARY_OBJECT_BODY_SUFFIX = ".body.bin";

    inline static const Container::String YAML_OBJECT_BODY_SUFFIX = ".body.yaml";

    LibraryLoader (TypeManifest _typeManifest) noexcept;

    LibraryLoader (const LibraryLoader &_other) = delete;

    LibraryLoader (LibraryLoader &&_other) = delete;

    ~LibraryLoader () noexcept;

    void Begin (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept;

    bool IsLoading () const noexcept;

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

    void FormFolderList () noexcept;

    void LoadObjectDeclaration (const Container::String &_objectFolder,
                                Memory::UniqueString _objectName,
                                bool _loadingAsDependency) noexcept;

    void FormObjectList () noexcept;

    void LoadObjectBody (std::size_t _indexInList) noexcept;

    std::atomic_flag loading;
    std::atomic_unsigned_lock_free bodiesLeftToLoad = 0u;

    Library currentLibrary;
    Container::Vector<LibraryLoadingTask> loadingTasks {GetAllocationGroup ()};

    Container::Vector<Container::String> folderList {GetAllocationGroup ()};
    Container::Vector<ObjectListItem> objectList {GetAllocationGroup ()};
    Container::HashMap<Memory::UniqueString, std::size_t> indexInObjectList {GetAllocationGroup()};

    Serialization::Binary::PatchBundleDeserializer binaryPatchBundleDeserializer;
    Serialization::Yaml::PatchBundleDeserializer yamlPatchBundleDeserializer;

    TypeManifest typeManifest;
    Serialization::FieldNameLookupCache declarationFieldLookupCache {Declaration::Reflect ().mapping};
};

struct FolderDependency final
{
    static constexpr std::size_t RELATIVE_PATH_MAX_LENGTH = 128u;

    std::array<char, RELATIVE_PATH_MAX_LENGTH> relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Asset::Object
