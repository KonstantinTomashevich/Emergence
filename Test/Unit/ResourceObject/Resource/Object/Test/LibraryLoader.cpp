#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Object/LibraryLoader.hpp>
#include <Resource/Object/Test/Helpers.hpp>
#include <Resource/Object/Test/Types.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Resource::Object::Test
{
static const char *const ENVIRONMENT_ROOT = "./Assets";
static const char *const ENVIRONMENT_MOUNT = "Assets";

enum class SerializationFormat
{
    BINARY = 0u,
    YAML
};

struct ObjectDefinition final
{
    Memory::UniqueString id;

    /// \details Relative to ::ENVIRONMENT_ROOT.
    Container::String folderPath;

    Object object;
    SerializationFormat format = SerializationFormat::YAML;
};

VirtualFileSystem::Context PrepareEnvironment (const Container::Vector<ObjectDefinition> &_objects)
{
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};
    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    std::filesystem::create_directories (ENVIRONMENT_ROOT);
    VirtualFileSystem::Context virtualFileSystem;
    REQUIRE (virtualFileSystem.Mount (virtualFileSystem.GetRoot (), {VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                     ENVIRONMENT_ROOT, ENVIRONMENT_MOUNT}));

    for (const ObjectDefinition &object : _objects)
    {
        const std::filesystem::path folderPath = rootPath / object.folderPath;
        std::filesystem::create_directories (folderPath);

        switch (object.format)
        {
        case SerializationFormat::BINARY:
        {
            std::filesystem::path objectPath = folderPath / EMERGENCE_BUILD_STRING (*object.id, ".bin");
            std::ofstream objectOutput {objectPath, std::ios::binary};
            Serialization::Binary::SerializeTypeName (objectOutput, Object::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (objectOutput, &object.object, Object::Reflect ().mapping);
            break;
        }

        case SerializationFormat::YAML:
        {
            std::filesystem::path objectPath = folderPath / EMERGENCE_BUILD_STRING (*object.id, ".yaml");
            std::ofstream objectOutput {objectPath, std::ios::binary};
            Serialization::Yaml::SerializeTypeName (objectOutput, Object::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (objectOutput, &object.object, Object::Reflect ().mapping);
            break;
        }
        }
    }

    return virtualFileSystem;
}
} // namespace Emergence::Resource::Object::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Object::Test;
using namespace Emergence::Resource::Object;
using namespace Emergence::Resource::Provider;

BEGIN_SUITE (LibraryLoader)

TEST_CASE (LoadTrivial)
{
    const Emergence::Memory::UniqueString firstObjectId {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectId {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {firstObjectId, "Objects", {{}, firstObjectChangelist}},
        {secondObjectId, "Objects", {{}, secondObjectChangelist}},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{firstObjectId}, {secondObjectId}});
    const Library::ObjectData *firstObjectData = library.Find (firstObjectId);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectId);

    REQUIRE (firstObjectData);
    REQUIRE (secondObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 2u);

    CHECK_EQUAL (firstObjectData->object.parent, ""_us);
    CheckChangelistEquality (firstObjectData->object.changelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadDifferentFolders)
{
    const Emergence::Memory::UniqueString firstObjectId {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectId {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {firstObjectId, "FolderOne", {{}, firstObjectChangelist}},
        {secondObjectId, "FolderTwo", {{}, secondObjectChangelist}},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{firstObjectId}, {secondObjectId}});
    const Library::ObjectData *firstObjectData = library.Find (firstObjectId);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectId);

    REQUIRE (firstObjectData);
    REQUIRE (secondObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 2u);

    CHECK_EQUAL (firstObjectData->object.parent, ""_us);
    CheckChangelistEquality (firstObjectData->object.changelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadOnlySelected)
{
    const Emergence::Memory::UniqueString firstObjectId {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectId {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {firstObjectId, "Objects", {{}, firstObjectChangelist}},
        {secondObjectId, "Objects", {{}, secondObjectChangelist}},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{secondObjectId}});
    const Library::ObjectData *firstObjectData = library.Find (firstObjectId);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectId);

    REQUIRE (!firstObjectData);
    REQUIRE (secondObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 1u);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadWithInjection)
{
    const Emergence::Memory::UniqueString firstObjectId {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectId {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (InjectionComponent {0u, firstObjectId})};

    const Emergence::Memory::UniqueString thirdObjectId {"Third"};
    Emergence::Container::Vector<ObjectComponent> thirdObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (InjectionComponent {0u, secondObjectId})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {firstObjectId, "Objects", {{}, firstObjectChangelist}},
        {secondObjectId, "Objects", {{}, secondObjectChangelist}},
        {thirdObjectId, "Objects", {{}, thirdObjectChangelist}},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{thirdObjectId}});
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 3u);

    const Library::ObjectData *firstObjectData = library.Find (firstObjectId);
    REQUIRE (firstObjectData);

    const Library::ObjectData *secondObjectData = library.Find (secondObjectId);
    REQUIRE (secondObjectData);

    const Library::ObjectData *thirdObjectData = library.Find (thirdObjectId);
    REQUIRE (thirdObjectData);

    CHECK_EQUAL (firstObjectData->object.parent, ""_us);
    CheckChangelistEquality (firstObjectData->object.changelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);

    CHECK_EQUAL (thirdObjectData->object.parent, ""_us);
    CheckChangelistEquality (thirdObjectData->object.changelist, thirdObjectChangelist);
    CHECK_EQUAL (thirdObjectData->loadedAsParent, false);
}

TEST_CASE (LoadTrivialBinary)
{
    const Emergence::Memory::UniqueString firstObjectId {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectId {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {firstObjectId, "Objects", {{}, firstObjectChangelist}, SerializationFormat::BINARY},
        {secondObjectId, "Objects", {{}, secondObjectChangelist}, SerializationFormat::BINARY},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{firstObjectId}, {secondObjectId}});
    const Library::ObjectData *firstObjectData = library.Find (firstObjectId);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectId);

    REQUIRE (firstObjectData);
    REQUIRE (secondObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 2u);

    CHECK_EQUAL (firstObjectData->object.parent, ""_us);
    CheckChangelistEquality (firstObjectData->object.changelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadInheritance)
{
    const Emergence::Memory::UniqueString baseObjectId {"Base"};
    Emergence::Container::Vector<ObjectComponent> baseObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    const Emergence::Memory::UniqueString firstDerivationObjectId {"FirstDerivation"};
    Emergence::Container::Vector<ObjectComponent> firstDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 30u, 0u})};

    const Emergence::Memory::UniqueString secondDerivationObjectId {"SecondDerivation"};
    Emergence::Container::Vector<ObjectComponent> secondDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 0u, 11u})};

    Emergence::VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment ({
        {baseObjectId, "Objects", {{}, baseObjectChangelist}},
        {firstDerivationObjectId, "Objects", {baseObjectId, firstDerivationObjectChangelist}},
        {secondDerivationObjectId, "Objects", {firstDerivationObjectId, secondDerivationObjectChangelist}},
    });

    ResourceProvider resourceProvider {&virtualFileSystem, GetResourceObjectMappingRegistry (),
                                       GetPatchableTypesMappingRegistry ()};
    REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             SourceOperationResponse::SUCCESSFUL);
    LibraryLoader loader {&resourceProvider, GetTypeManifest ()};

    Library library = loader.Load ({{secondDerivationObjectId}});
    const Library::ObjectData *firstDerivationObjectData = library.Find (firstDerivationObjectId);
    const Library::ObjectData *secondDerivationObjectData = library.Find (secondDerivationObjectId);

    REQUIRE (firstDerivationObjectData);
    REQUIRE (secondDerivationObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 3u);

    CHECK_EQUAL (firstDerivationObjectData->object.parent, baseObjectId);
    ObjectComponent firstDerivationComponent {baseObjectChangelist.front ().component +
                                              firstDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (firstDerivationObjectData->object.changelist, {firstDerivationComponent});
    CHECK_EQUAL (firstDerivationObjectData->loadedAsParent, true);

    CHECK_EQUAL (secondDerivationObjectData->object.parent, firstDerivationObjectId);
    ObjectComponent secondDerivationComponent {firstDerivationComponent.component +
                                               secondDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (secondDerivationObjectData->object.changelist, {secondDerivationComponent});
    CHECK_EQUAL (secondDerivationObjectData->loadedAsParent, false);
}

END_SUITE
