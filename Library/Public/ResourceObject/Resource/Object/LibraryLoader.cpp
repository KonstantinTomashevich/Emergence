#include <filesystem>
#include <fstream>

#include <Assert/Assert.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Resource/Object/LibraryLoader.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Resource::Object
{
LibraryLoader::LibraryLoader (TypeManifest _typeManifest) noexcept
    : typeManifest (std::move (_typeManifest))
{
    for (const auto &mappingInfoPair : typeManifest.GetMap ())
    {
        patchableTypesRegistry.Register (mappingInfoPair.first);
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
        Job::Priority::BACKGROUND,
        [this] ()
        {
            loadingStartTimeNs = Time::NanosecondsSinceStartup ();
            FormFolderList ();
            FormObjectList ();

            size_t objectIndex = 0u;
            while (objectIndex < objectList.size ())
            {
                PostProcessObject (objectIndex);
                ++objectIndex;
            }

            loading.clear (std::memory_order_release);
            const uint64_t loadingTimeNs = Time::NanosecondsSinceStartup () - loadingStartTimeNs;
            EMERGENCE_LOG (INFO, "Resource::Object::LibraryLoader: Library loading took ",
                           static_cast<float> (loadingTimeNs) * 1e-9f, " seconds.");
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
    FolderDependencyList folderDependencyList;

    std::filesystem::path binDependencyListPath = folderPath / BINARY_FOLDER_DEPENDENCY_LIST;
    std::filesystem::path yamlDependencyListPath = folderPath / YAML_FOLDER_DEPENDENCY_LIST;

    if (std::filesystem::is_regular_file (binDependencyListPath))
    {
        std::ifstream input (binDependencyListPath, std::ios::binary);
        // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
        input.get ();
        input.unget ();

        if (input)
        {
            if (Serialization::Binary::DeserializeObject (input, &folderDependencyList,
                                                          FolderDependencyList::Reflect ().mapping, {}))
            {
                for (const FolderDependency &dependency : folderDependencyList.list)
                {
                    RegisterFolder ((folderPath / dependency.relativePath.data ())
                                        .lexically_normal ()
                                        .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ());
                }
            }
            else
            {
                EMERGENCE_LOG (
                    ERROR, "Resource::Object::LibraryLoader: Unable to deserialize folder dependency list \"",
                    binDependencyListPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                    "\".");
            }
        }
    }
    else if (std::filesystem::is_regular_file (yamlDependencyListPath))
    {
        std::ifstream input (yamlDependencyListPath);
        // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
        input.get ();
        input.unget ();

        if (input)
        {
            if (Serialization::Yaml::DeserializeObject (input, &folderDependencyList,
                                                        FolderDependencyList::Reflect ().mapping, {}))
            {
                for (const FolderDependency &dependency : folderDependencyList.list)
                {
                    RegisterFolder ((folderPath / dependency.relativePath.data ())
                                        .lexically_normal ()
                                        .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ());
                }
            }
            else
            {
                EMERGENCE_LOG (
                    ERROR, "Resource::Object::LibraryLoader: Unable to deserialize folder dependency list \"",
                    yamlDependencyListPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                    "\".");
            }
        }
    }
    else
    {
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to find dependencies list for folder \"",
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
}

bool LibraryLoader::FindAndLoadObject (Memory::UniqueString _objectName, bool _loadingAsParent) noexcept
{
    for (const Container::String &folder : folderList)
    {
        std::filesystem::path parentBinaryDeclarationPath {
            EMERGENCE_BUILD_STRING (folder, "/", _objectName, BINARY_OBJECT_SUFFIX)};

        std::filesystem::path parentYamlDeclarationPath {
            EMERGENCE_BUILD_STRING (folder, "/", _objectName, YAML_OBJECT_SUFFIX)};

        if (std::filesystem::is_regular_file (parentBinaryDeclarationPath) ||
            std::filesystem::is_regular_file (parentYamlDeclarationPath))
        {
            LoadObject (folder, _objectName, _loadingAsParent);
            return true;
        }
    }

    return false;
}

void LibraryLoader::LoadObject (const Container::String &_objectFolder,
                                Memory::UniqueString _objectName,
                                bool _loadingAsParent) noexcept
{
    if (auto iterator = currentLibrary.objects.find (_objectName); iterator != currentLibrary.objects.end ())
    {
        auto indexInObjectListIterator = indexInObjectList.find (_objectName);
        if (indexInObjectListIterator == indexInObjectList.end ())
        {
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Found cyclic dependency during object \"",
                           _objectName, "\" parent traversal!");
        }
        else
        {
            // Check that we're trying to read the same object, not its name-duplicate.
            ObjectListItem &objectListItem = objectList[indexInObjectListIterator->second];
            iterator->second.loadedAsParent &= _loadingAsParent;

            if (_objectFolder != objectListItem.folder)
            {
                EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Both folders \"", _objectFolder, "\" and \"",
                               objectListItem.folder, "\" contain object \"", _objectName,
                               "\". That results in undefined behaviour!");
            }
        }

        return;
    }

    Object object;
    std::filesystem::path binaryPath {EMERGENCE_BUILD_STRING (_objectFolder, "/", _objectName, BINARY_OBJECT_SUFFIX)};

    std::filesystem::path yamlPath {EMERGENCE_BUILD_STRING (_objectFolder, "/", _objectName, YAML_OBJECT_SUFFIX)};

    if (std::filesystem::is_regular_file (binaryPath))
    {
        std::ifstream input {binaryPath, std::ios::binary};
        Serialization::Binary::DeserializeObject (input, &object, Object::Reflect ().mapping, patchableTypesRegistry);
    }
    else if (std::filesystem::is_regular_file (yamlPath))
    {
        std::ifstream input {yamlPath};
        Serialization::Yaml::DeserializeObject (input, &object, Object::Reflect ().mapping, patchableTypesRegistry);
    }
    else
    {
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to find declaration for object \"", _objectName,
                       "\" in folder \"", _objectFolder, "\".");
        return;
    }

    Memory::UniqueString parent = object.parent;
    currentLibrary.objects.emplace (_objectName, Library::ObjectData {std::move (object), _loadingAsParent});

    if (*parent)
    {
        if (!FindAndLoadObject (parent, true))
        {
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to find parent \"", parent,
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
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to find folder \"",
                           folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\"!");
            continue;
        }

        if (*task.loadSelectedObject)
        {
            LoadObject (folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
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

            if (relativePath.ends_with (BINARY_OBJECT_SUFFIX))
            {
                objectName = Memory::UniqueString {
                    relativePath.substr (0u, relativePath.size () - BINARY_OBJECT_SUFFIX.size ())};
            }
            else if (relativePath.ends_with (YAML_OBJECT_SUFFIX))
            {
                objectName =
                    Memory::UniqueString {relativePath.substr (0u, relativePath.size () - YAML_OBJECT_SUFFIX.size ())};
            }

            // If this file is not a declaration, objectName will be left empty, so we will ignore this file.
            if (*objectName)
            {
                LoadObject (folderPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                            objectName, false);
            }
        }
    }

    if (objectList.empty ())
    {
        EMERGENCE_LOG (WARNING, "Resource::Object::LibraryLoader: There is no objects that can be loaded!");
    }
}

void LibraryLoader::PostProcessObject (std::size_t _indexInList) noexcept
{
    auto objectDataIterator = currentLibrary.objects.find (objectList[_indexInList].name);
    if (objectDataIterator == currentLibrary.objects.end ())
    {
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Internal logic error occurred. Object \"",
                       objectList[_indexInList].name,
                       "\" is found in object list, but its declaration is not found in library.");
        return;
    }

    Library::ObjectData &objectData = objectDataIterator->second;
    if (*objectData.object.parent)
    {
        auto parentObjectDataIterator = currentLibrary.objects.find (objectData.object.parent);
        if (parentObjectDataIterator == currentLibrary.objects.end ())
        {
            EMERGENCE_LOG (
                ERROR, "Resource::Object::LibraryLoader: Internal logic error occurred. Parent for object \"",
                objectList[_indexInList].name, "\", which is \"", objectData.object.parent, "\", is not found.");
            return;
        }

        Library::ObjectData &parentObjectData = parentObjectDataIterator->second;
        ApplyInheritance (typeManifest, parentObjectData.object, objectData.object);
    }

    // Scan loaded object data for injections. Skip this scan for dependency
    // objects, as their injections might be overridden by their children.
    if (!objectData.loadedAsParent)
    {
        for (const ObjectComponent &component : objectData.object.changelist)
        {
            for (const DependencyInjectionInfo &injection : typeManifest.GetInjections ())
            {
                if (component.component.GetTypeMapping () == injection.injectorType)
                {
                    for (const StandardLayout::Patch::ChangeInfo &change : component.component)
                    {
                        if (change.field == injection.injectorIdField)
                        {
                            const Memory::UniqueString dependency =
                                *static_cast<const Memory::UniqueString *> (change.newValue);

                            if (!FindAndLoadObject (dependency, false))
                            {
                                EMERGENCE_LOG (ERROR,
                                               "Resource::Object::LibraryLoader: Unable to find injected sub object \"",
                                               dependency, "\" for object \"", objectList[_indexInList].name, "\".");
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

const FolderDependency::Reflection &FolderDependency::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FolderDependency);
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const FolderDependencyList::Reflection &FolderDependencyList::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FolderDependencyList);
        EMERGENCE_MAPPING_REGISTER_REGULAR (list);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Resource::Object

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Resource::Object::LibraryLoadingTask>::Get () noexcept
{
    static Profiler::AllocationGroup group {Resource::Object::GetRootAllocationGroup (),
                                            UniqueString {"LibraryLoadingTask"}};
    return group;
}
} // namespace Emergence::Memory
