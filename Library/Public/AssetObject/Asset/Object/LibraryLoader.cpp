#include <filesystem>
#include <fstream>

#include <Assert/Assert.hpp>

#include <Asset/Object/LibraryLoader.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/OnScopeExit.hpp>

namespace Emergence::Asset::Object
{
LibraryLoader::LibraryLoader (TypeManifest _typeManifest) noexcept
    : typeManifest (std::move (_typeManifest))
{
    for (const auto &mappingInfoPair : typeManifest.GetMap ())
    {
        binaryPatchBundleDeserializer.RegisterType (mappingInfoPair.first);
        yamlPatchBundleDeserializer.RegisterType (mappingInfoPair.first);
    }
}

LibraryLoader::~LibraryLoader () noexcept
{
    EMERGENCE_ASSERT (!IsLoading ());
}

void LibraryLoader::Begin (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept
{
    EMERGENCE_ASSERT (!IsLoading ());
    loading.test_and_set (std::memory_order_acquire);

    for (const LibraryLoadingTask &task : _loadingTasks)
    {
        loadingTasks.emplace_back (task);
    }

    Job::Dispatcher::Global ().Dispatch (
        [this] ()
        {
            FormFolderList ();
        });
}

bool LibraryLoader::IsLoading () const noexcept
{
    return loading.test (std::memory_order_acquire);
}

const Container::Vector<LibraryLoadingTask> &LibraryLoader::GetLoadingTasks () noexcept
{
    return loadingTasks;
}

Library LibraryLoader::End () noexcept
{
    EMERGENCE_ASSERT (!IsLoading ());
    loadingTasks.clear ();
    folderList.clear ();
    objectList.clear ();
    return std::move (currentLibrary);
}

Memory::Profiler::AllocationGroup LibraryLoader::GetAllocationGroup () noexcept
{
    return Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"LibraryLoader"}};
}

void LibraryLoader::RegisterFolder (const Container::String &_folder) noexcept
{
    if (std::find (folderList.begin (), folderList.end (), _folder) != folderList.end ())
    {
        return;
    }

    folderList.emplace_back (_folder);
    std::filesystem::path folderPath {_folder};
    FolderDependency dependency;

    std::filesystem::path binDependencyListPath = folderPath / BINARY_FOLDER_DEPENDENCY_LIST;
    std::filesystem::path yamlDependencyListPath = folderPath / YAML_FOLDER_DEPENDENCY_LIST;

    if (std::filesystem::is_regular_file (binDependencyListPath))
    {
        std::ifstream input (binDependencyListPath, std::ios::binary);
        // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
        input.get ();
        input.unget ();

        while (input)
        {
            if (!Serialization::Binary::DeserializeObject (input, &dependency, FolderDependency::Reflect ().mapping))
            {
                EMERGENCE_LOG (
                    ERROR, "Asset::Object::LibraryLoader: Unable to deserialize folder dependencies list \"",
                    binDependencyListPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                    "\".");
                break;
            }

            RegisterFolder ((folderPath / dependency.relativePath.data ())
                                .lexically_normal ()
                                .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ());

            // Use peek to test for the end of file or other problems in given stream.
            input.peek ();
        }
    }
    else if (std::filesystem::is_regular_file (yamlDependencyListPath))
    {
        std::ifstream input (yamlDependencyListPath);
        Serialization::Yaml::ObjectBundleDeserializer deserializer {FolderDependency::Reflect ().mapping};
        bool successful = deserializer.Begin (input);

        while (successful && deserializer.HasNext ())
        {
            if ((successful = deserializer.Next (&dependency)))
            {
                RegisterFolder ((folderPath / dependency.relativePath.data ())
                                    .lexically_normal ()
                                    .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ());
            }
        }

        deserializer.End ();
        if (!successful)
        {
            EMERGENCE_LOG (
                ERROR, "Asset::Object::LibraryLoader: Unable to deserialize folder dependencies list \"",
                yamlDependencyListPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
        }
    }
    else
    {
        EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to find dependencies list for folder \"",
                       folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
    }
}

void LibraryLoader::FormFolderList () noexcept
{
    for (const LibraryLoadingTask &task : loadingTasks)
    {
        std::filesystem::path folderPath {task.folder};
        RegisterFolder (
            folderPath.lexically_normal ().generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ());
    }

    Job::Dispatcher::Global ().Dispatch (
        [this] ()
        {
            FormObjectList ();
        });
}

void LibraryLoader::LoadObjectDeclaration (const Container::String &_objectFolder,
                                           Memory::UniqueString _objectName,
                                           bool _loadingAsDependency) noexcept
{
    if (auto iterator = currentLibrary.objects.find (_objectName); iterator != currentLibrary.objects.end ())
    {
        auto indexInObjectListIterator = indexInObjectList.find (_objectName);
        if (indexInObjectListIterator == indexInObjectList.end ())
        {
            EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Found cyclic dependency during object \"", _objectName,
                           "\" parent traversal!");
        }
        else
        {
            // Check that we're trying to read the same object, not its name-duplicate.
            ObjectListItem &objectListItem = objectList[indexInObjectListIterator->second];
            iterator->second.loadedAsDependency &= _loadingAsDependency;

            if (_objectFolder != objectListItem.folder)
            {
                EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Both folders \"", _objectFolder, "\" and \"",
                               objectListItem.folder, "\" contain object \"", _objectName,
                               "\". That results in undefined behaviour!");
            }
        }

        return;
    }

    Declaration declaration;
    std::filesystem::path binaryDeclarationPath {
        EMERGENCE_BUILD_STRING (_objectFolder, "/", _objectName, BINARY_OBJECT_DECLARATION_SUFFIX)};

    std::filesystem::path yamlDeclarationPath {
        EMERGENCE_BUILD_STRING (_objectFolder, "/", _objectName, YAML_OBJECT_DECLARATION_SUFFIX)};

    if (std::filesystem::is_regular_file (binaryDeclarationPath))
    {
        std::ifstream input {binaryDeclarationPath, std::ios::binary};
        Serialization::Binary::DeserializeObject (input, &declaration, Declaration::Reflect ().mapping);
    }
    else if (std::filesystem::is_regular_file (yamlDeclarationPath))
    {
        std::ifstream input {yamlDeclarationPath};
        Serialization::Yaml::DeserializeObject (input, &declaration, declarationFieldLookupCache);
    }
    else
    {
        EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to find declaration for object \"", _objectName,
                       "\" in folder \"", _objectFolder, "\".");
        return;
    }

    currentLibrary.objects.emplace (_objectName, Library::ObjectData {declaration, {}, _loadingAsDependency});
    if (*declaration.parent)
    {
        bool parentFound = false;
        for (const Container::String &folder : folderList)
        {
            std::filesystem::path parentBinaryDeclarationPath {
                EMERGENCE_BUILD_STRING (folder, "/", declaration.parent, BINARY_OBJECT_DECLARATION_SUFFIX)};

            std::filesystem::path parentYamlDeclarationPath {
                EMERGENCE_BUILD_STRING (folder, "/", declaration.parent, YAML_OBJECT_DECLARATION_SUFFIX)};

            if (std::filesystem::is_regular_file (parentBinaryDeclarationPath) ||
                std::filesystem::is_regular_file (parentYamlDeclarationPath))
            {
                LoadObjectDeclaration (folder, declaration.parent, true);
                parentFound = true;
                break;
            }
        }

        if (!parentFound)
        {
            EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to find parent \"", declaration.parent,
                           "\" for object \"", _objectName, "\".");
        }
    }

    indexInObjectList.emplace (_objectName, objectList.size ());
    objectList.emplace_back () = {_objectFolder, _objectName};
}

void LibraryLoader::FormObjectList () noexcept
{
    for (const LibraryLoadingTask &task : loadingTasks)
    {
        std::filesystem::path folderPath = std::filesystem::path {task.folder}.lexically_normal ();
        if (!std::filesystem::exists (folderPath))
        {
            EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to find folder \"",
                           folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\"!");
            continue;
        }

        if (*task.loadSelectedObject)
        {
            LoadObjectDeclaration (folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                   task.loadSelectedObject, false);
            continue;
        }

        for (const std::filesystem::directory_entry &entry : std::filesystem::recursive_directory_iterator (folderPath))
        {
            if (!std::filesystem::is_regular_file (entry))
            {
                continue;
            }

            Container::String relativePath =
                std::filesystem::relative (entry.path (), folderPath)
                    .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
            Memory::UniqueString objectName;

            if (relativePath.ends_with (BINARY_OBJECT_DECLARATION_SUFFIX))
            {
                objectName = Memory::UniqueString {
                    relativePath.substr (0u, relativePath.size () - BINARY_OBJECT_DECLARATION_SUFFIX.size ())};
            }
            else if (relativePath.ends_with (YAML_OBJECT_DECLARATION_SUFFIX))
            {
                objectName = Memory::UniqueString {
                    relativePath.substr (0u, relativePath.size () - YAML_OBJECT_DECLARATION_SUFFIX.size ())};
            }

            // If this file is not a declaration, objectName will be left empty, so we will ignore this file.
            if (*objectName)
            {
                LoadObjectDeclaration (
                    folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), objectName,
                    false);
            }
        }
    }

    if (objectList.empty ())
    {
        EMERGENCE_LOG (WARNING, "Asset::Object::LibraryLoader: There is no objects that can be loaded!");
    }

    Job::Dispatcher::Global ().Dispatch (
        [this] ()
        {
            LoadObjectBody (0u);
        });
}

void LibraryLoader::LoadObjectBody (std::size_t _indexInList) noexcept
{
    if (_indexInList >= objectList.size ())
    {
        loading.clear (std::memory_order_release);
        return;
    }

    EMERGENCE_ON_SCOPE_EXIT (
        [this, _indexInList] ()
        {
            Job::Dispatcher::Global ().Dispatch (
                [this, _indexInList] ()
                {
                    LoadObjectBody (_indexInList + 1u);
                });
        });

    ObjectListItem &objectItem = objectList[_indexInList];
    auto objectDataIterator = currentLibrary.objects.find (objectItem.name);

    if (objectDataIterator == currentLibrary.objects.end ())
    {
        EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Internal logic error occurred. Object \"", objectItem.name,
                       "\" is found in object list, but its declaration is not found in library.");
        return;
    }

    Library::ObjectData &objectData = objectDataIterator->second;
    std::filesystem::path binaryBodyPath {
        EMERGENCE_BUILD_STRING (objectItem.folder, "/", objectItem.name, BINARY_OBJECT_BODY_SUFFIX)};

    std::filesystem::path yamlBodyPath {
        EMERGENCE_BUILD_STRING (objectItem.folder, "/", objectItem.name, YAML_OBJECT_BODY_SUFFIX)};

    Container::Vector<StandardLayout::Patch> changelist {GetAllocationGroup ()};
    if (std::filesystem::is_regular_file (binaryBodyPath))
    {
        std::ifstream input {binaryBodyPath, std::ios::binary};
        binaryPatchBundleDeserializer.Begin (input);

        while (binaryPatchBundleDeserializer.HasNext ())
        {
            if (Container::Optional<StandardLayout::Patch> nextPatch = binaryPatchBundleDeserializer.Next ())
            {
                changelist.emplace_back (nextPatch.value ());
            }
            else
            {
                EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to load object body from \"",
                               binaryBodyPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                break;
            }
        }

        binaryPatchBundleDeserializer.End ();
    }
    else if (std::filesystem::is_regular_file (yamlBodyPath))
    {
        std::ifstream input {yamlBodyPath};
        bool successful = yamlPatchBundleDeserializer.Begin (input);

        while (successful && yamlPatchBundleDeserializer.HasNext ())
        {
            if (Container::Optional<StandardLayout::Patch> nextPatch = yamlPatchBundleDeserializer.Next ();
                (successful &= nextPatch.has_value ()))
            {
                changelist.emplace_back (nextPatch.value ());
            }
        }

        yamlPatchBundleDeserializer.End ();
        if (!successful)
        {
            EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to load object body from \"",
                           yamlBodyPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
        }
    }
    else
    {
        EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Unable to find body for object \"", objectItem.name,
                       "\" in folder \"", objectItem.folder, "\".");
        return;
    }

    if (*objectData.declaration.parent)
    {
        auto parentObjectDataIterator = currentLibrary.objects.find (objectData.declaration.parent);
        if (parentObjectDataIterator == currentLibrary.objects.end ())
        {
            EMERGENCE_LOG (ERROR, "Asset::Object::LibraryLoader: Internal logic error occurred. Parent for object \"",
                           objectItem.name, "\", which is \"", objectData.declaration.parent, "\", is not found.");
            return;
        }

        Library::ObjectData &parentObjectData = parentObjectDataIterator->second;
        objectData.body = ApplyInheritance (typeManifest, parentObjectData.body, changelist);
    }
    else
    {
        for (const StandardLayout::Patch &patch : changelist)
        {
            objectData.body.fullChangelist.emplace_back (patch);
        }
    }
}

const FolderDependency::Reflection &FolderDependency::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FolderDependency);
        EMERGENCE_MAPPING_REGISTER_STRING (relativePath);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Asset::Object

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Asset::Object::LibraryLoadingTask>::Get () noexcept
{
    static Profiler::AllocationGroup group {Asset::Object::GetRootAllocationGroup (),
                                            UniqueString {"LibraryLoadingTask"}};
    return group;
}
} // namespace Emergence::Memory
