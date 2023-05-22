#include <Log/Log.hpp>

#include <Resource/Object/LibraryLoader.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Resource::Object
{
LibraryLoader::LibraryLoader (ResourceProvider::ResourceProvider *_resourceProvider,
                              TypeManifest _typeManifest) noexcept
    : resourceProvider (_resourceProvider),
      typeManifest (std::move (_typeManifest))
{
}

Library LibraryLoader::Load (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept
{
    uint64_t loadingStartTimeNs = Time::NanosecondsSinceStartup ();
    FormObjectList (_loadingTasks);
    size_t objectIndex = 0u;

    while (objectIndex < objectList.size ())
    {
        PostProcessObject (objectIndex);
        ++objectIndex;
    }

    const uint64_t loadingTimeNs = Time::NanosecondsSinceStartup () - loadingStartTimeNs;
    EMERGENCE_LOG (INFO, "Resource::Object::LibraryLoader: Library loading took ",
                   static_cast<float> (loadingTimeNs) * 1e-9f, " seconds.");

    objectList.clear ();
    indexInObjectList.clear();
    return std::move (currentLibrary);
}

Memory::Profiler::AllocationGroup LibraryLoader::GetAllocationGroup () noexcept
{
    return Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"LibraryLoader"}};
}

bool LibraryLoader::LoadObject (Memory::UniqueString _objectId, bool _loadingAsParent) noexcept
{
    if (auto iterator = currentLibrary.objects.find (_objectId); iterator != currentLibrary.objects.end ())
    {
        auto indexInObjectListIterator = indexInObjectList.find (_objectId);
        if (indexInObjectListIterator == indexInObjectList.end ())
        {
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Found cyclic dependency during object \"",
                           _objectId, "\" parent traversal!");
        }
        else
        {
            iterator->second.loadedAsParent &= _loadingAsParent;
        }

        return true;
    }

    Object object;
    switch (resourceProvider->LoadObject (Object::Reflect ().mapping, _objectId, &object))
    {
    case ResourceProvider::LoadingOperationResponse::SUCCESSFUL:
        break;

    case ResourceProvider::LoadingOperationResponse::NOT_FOUND:
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Object \"", _objectId, "\" is not found.");
        return false;

    case ResourceProvider::LoadingOperationResponse::IO_ERROR:
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to load object \"", _objectId,
                       "\" due to an IO error.");
        return false;

    case ResourceProvider::LoadingOperationResponse::WRONG_TYPE:
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to load object \"", _objectId,
                       "\", there is another resource with the same id.");
        return false;
    }

    Memory::UniqueString parent = object.parent;
    currentLibrary.objects.emplace (_objectId, Library::ObjectData {std::move (object), _loadingAsParent});

    if (*parent)
    {
        if (!LoadObject (parent, true))
        {
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to load parent \"", parent,
                           "\" for object \"", _objectId, "\".");
            currentLibrary.objects.erase (_objectId);
            return false;
        }
    }

    indexInObjectList.emplace (_objectId, objectList.size ());
    objectList.emplace_back (_objectId);
    return true;
}

void LibraryLoader::FormObjectList (const Container::Vector<LibraryLoadingTask> &_loadingTasks) noexcept
{
    for (const LibraryLoadingTask &task : _loadingTasks)
    {
        if (!LoadObject (task.objectId, false))
        {
            EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Unable to load requested object \"", task.objectId,
                           "\".");
        }
    }

    if (objectList.empty ())
    {
        EMERGENCE_LOG (WARNING, "Resource::Object::LibraryLoader: There is no objects that can be loaded!");
    }
}

void LibraryLoader::PostProcessObject (std::size_t _indexInList) noexcept
{
    auto objectDataIterator = currentLibrary.objects.find (objectList[_indexInList]);
    if (objectDataIterator == currentLibrary.objects.end ())
    {
        EMERGENCE_LOG (ERROR, "Resource::Object::LibraryLoader: Internal logic error occurred. Object \"",
                       objectList[_indexInList], "\" is found in object list, but its data is not found in library.");
        return;
    }

    Library::ObjectData &objectData = objectDataIterator->second;
    if (*objectData.object.parent)
    {
        auto parentObjectDataIterator = currentLibrary.objects.find (objectData.object.parent);
        if (parentObjectDataIterator == currentLibrary.objects.end ())
        {
            EMERGENCE_LOG (ERROR,
                           "Resource::Object::LibraryLoader: Internal logic error occurred. Parent for object \"",
                           objectList[_indexInList], "\", which is \"", objectData.object.parent, "\", is not found.");
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

                            if (!LoadObject (dependency, false))
                            {
                                EMERGENCE_LOG (ERROR,
                                               "Resource::Object::LibraryLoader: Unable to load injected sub object \"",
                                               dependency, "\" for object \"", objectList[_indexInList], "\".");
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
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
