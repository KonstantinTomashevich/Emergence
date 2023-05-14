#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Container/String.hpp>
#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Object/LibraryLoader.hpp>
#include <Resource/Object/Test/Helpers.hpp>
#include <Resource/Object/Test/Types.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Resource::Object::Test
{
static const char *const ENVIRONMENT_ROOT = "./Assets";

enum class SerializationFormat
{
    BINARY = 0u,
    YAML
};

struct ObjectDefinition final
{
    Memory::UniqueString name;
    Object object;
    SerializationFormat format = SerializationFormat::YAML;
};

struct FolderDefinition final
{
    /// \details Relative to ::ENVIRONMENT_ROOT.
    Container::String relativePath;

    /// \details Relative to this folder path.
    FolderDependencyList dependencyList;

    SerializationFormat dependencySerializationFormat = SerializationFormat::YAML;

    Container::Vector<ObjectDefinition> objects;
};

void PrepareEnvironment (const Container::Vector<FolderDefinition> &_folders)
{
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};
    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    for (const FolderDefinition &folder : _folders)
    {
        const std::filesystem::path folderPath = rootPath / folder.relativePath;
        std::filesystem::create_directories (folderPath);

        FolderDependency dependency;
        const bool eitherBinaryOrHasDependencies =
            folder.dependencySerializationFormat == SerializationFormat::BINARY || !folder.dependencyList.list.empty ();
        REQUIRE_WITH_MESSAGE (eitherBinaryOrHasDependencies, "Only binary format supports absence of dependencies!");

        switch (folder.dependencySerializationFormat)
        {
        case SerializationFormat::BINARY:
        {
            std::ofstream output {folderPath / LibraryLoader::BINARY_FOLDER_DEPENDENCY_LIST, std::ios::binary};
            if (!folder.dependencyList.list.empty ())
            {
                Serialization::Binary::SerializeObject (output, &folder.dependencyList,
                                                        FolderDependencyList::Reflect ().mapping);
            }

            break;
        }
        case SerializationFormat::YAML:
        {
            std::ofstream output {folderPath / LibraryLoader::YAML_FOLDER_DEPENDENCY_LIST};
            Serialization::Yaml::SerializeObject (output, &folder.dependencyList,
                                                  FolderDependencyList::Reflect ().mapping);
            break;
        }
        }

        for (const ObjectDefinition &object : folder.objects)
        {
            std::filesystem::path objectPath = folderPath / *object.name;
            std::filesystem::create_directories (objectPath.parent_path ());

            switch (object.format)
            {
            case SerializationFormat::BINARY:
            {
                std::ofstream objectOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::BINARY_OBJECT_SUFFIX)
                        .c_str (),
                    std::ios::binary};

                Serialization::Binary::SerializeObject (objectOutput, &object.object, Object::Reflect ().mapping);
                break;
            }

            case SerializationFormat::YAML:
            {
                std::ofstream objectOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::YAML_OBJECT_SUFFIX)
                        .c_str (),
                    std::ios::binary};

                Serialization::Yaml::SerializeObject (objectOutput, &object.object, Object::Reflect ().mapping);
                break;
            }
            }
        }
    }
}
} // namespace Emergence::Resource::Object::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Object::Test;
using namespace Emergence::Resource::Object;

BEGIN_SUITE (LibraryLoader)

TEST_CASE (LoadTrivial)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}},
                              {secondObjectName, {{}, secondObjectChangelist}},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), {}}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *firstObjectData = library.Find (firstObjectName);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);

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

TEST_CASE (LoadSpecified)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}},
                              {secondObjectName, {{}, secondObjectChangelist}},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), secondObjectName}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);
    REQUIRE (secondObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 1u);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadSpecifiedWithInjection)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (InjectionComponent {0u, firstObjectName})};

    const Emergence::Memory::UniqueString thirdObjectName {"Third"};
    Emergence::Container::Vector<ObjectComponent> thirdObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (InjectionComponent {0u, secondObjectName})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}},
                              {secondObjectName, {{}, secondObjectChangelist}},
                              {thirdObjectName, {{}, thirdObjectChangelist}},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), thirdObjectName}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 3u);

    const Library::ObjectData *firstObjectData = library.Find (firstObjectName);
    REQUIRE (firstObjectData);

    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);
    REQUIRE (secondObjectData);

    const Library::ObjectData *thirdObjectData = library.Find (thirdObjectName);
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

TEST_CASE (InjectionFromOtherFolder)
{
    const Emergence::Memory::UniqueString firstFolderName {"Objects/First"};
    const Emergence::Memory::UniqueString secondFolderName {"Objects/Second"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (InjectionComponent {0u, firstObjectName})};

    PrepareEnvironment ({{*firstFolderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}},
                          }},
                         {*secondFolderName,
                          {{{"../First"}}},
                          SerializationFormat::BINARY,
                          {
                              {secondObjectName, {{}, secondObjectChangelist}},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", secondFolderName), secondObjectName}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 2u);

    const Library::ObjectData *firstObjectData = library.Find (firstObjectName);
    REQUIRE (firstObjectData);

    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);
    REQUIRE (secondObjectData);

    CHECK_EQUAL (firstObjectData->object.parent, ""_us);
    CheckChangelistEquality (firstObjectData->object.changelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondObjectData->object.parent, ""_us);
    CheckChangelistEquality (secondObjectData->object.changelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsParent, false);
}

TEST_CASE (LoadTrivialBinary)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}, SerializationFormat::BINARY},
                              {secondObjectName, {{}, secondObjectChangelist}, SerializationFormat::BINARY},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), {}}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *firstObjectData = library.Find (firstObjectName);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);

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

TEST_CASE (LoadTrivialSubdirectories)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"Subdir1/First"};
    Emergence::Container::Vector<ObjectComponent> firstObjectChangelist {
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Subdir2/Second"};
    Emergence::Container::Vector<ObjectComponent> secondObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {{}, firstObjectChangelist}},
                              {secondObjectName, {{}, secondObjectChangelist}},
                          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), {}}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *firstObjectData = library.Find (firstObjectName);
    const Library::ObjectData *secondObjectData = library.Find (secondObjectName);

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
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString baseObjectName {"Base"};
    Emergence::Container::Vector<ObjectComponent> baseObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u})};

    const Emergence::Memory::UniqueString firstDerivationObjectName {"FirstDerivation"};
    Emergence::Container::Vector<ObjectComponent> firstDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 30u, 0u})};

    const Emergence::Memory::UniqueString secondDerivationObjectName {"SecondDerivation"};
    Emergence::Container::Vector<ObjectComponent> secondDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 0u, 11u})};

    PrepareEnvironment (
        {{*folderName,
          {},
          SerializationFormat::BINARY,
          {
              {baseObjectName, {{}, baseObjectChangelist}},
              {firstDerivationObjectName, {baseObjectName, firstDerivationObjectChangelist}},
              {secondDerivationObjectName, {firstDerivationObjectName, secondDerivationObjectChangelist}},
          }}});

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", folderName), {}}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *firstDerivationObjectData = library.Find (firstDerivationObjectName);
    const Library::ObjectData *secondDerivationObjectData = library.Find (secondDerivationObjectName);

    REQUIRE (firstDerivationObjectData);
    REQUIRE (secondDerivationObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 3u);

    CHECK_EQUAL (firstDerivationObjectData->object.parent, baseObjectName);
    ObjectComponent firstDerivationComponent {baseObjectChangelist.front ().component +
                                              firstDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (firstDerivationObjectData->object.changelist, {firstDerivationComponent});
    CHECK_EQUAL (firstDerivationObjectData->loadedAsParent, false);

    CHECK_EQUAL (secondDerivationObjectData->object.parent, firstDerivationObjectName);
    ObjectComponent secondDerivationComponent {firstDerivationComponent.component +
                                               secondDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (secondDerivationObjectData->object.changelist, {secondDerivationComponent});
    CHECK_EQUAL (secondDerivationObjectData->loadedAsParent, false);
}

TEST_CASE (LoadDependencies)
{
    const Emergence::Memory::UniqueString baseFolderName {"Objects/Common"};
    const Emergence::Memory::UniqueString firstDerivationFolderName {"Objects/Units"};
    const Emergence::Memory::UniqueString secondDerivationFolderName {"Objects/Maps/Ranglor"};

    const Emergence::Memory::UniqueString baseObjectName {"Base"};
    Emergence::Container::Vector<ObjectComponent> baseObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 100u, 20u, 10u}),
        MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 0.5f})};

    const Emergence::Memory::UniqueString firstDerivationObjectName {"FirstDerivation"};
    Emergence::Container::Vector<ObjectComponent> firstDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 30u, 0u}),
        MakeComponentPatch (FirstComponent {1u, 5.0f, 3.0f, 4.0f, 1.0f})};

    const Emergence::Memory::UniqueString secondDerivationObjectName {"SecondDerivation"};
    Emergence::Container::Vector<ObjectComponent> secondDerivationObjectChangelist {
        MakeComponentPatch (SecondComponent {0u, 0u, 0u, 11u})};

    PrepareEnvironment ({
        {*baseFolderName,
         {},
         SerializationFormat::BINARY,
         {
             {baseObjectName, {{}, baseObjectChangelist}},
         }},
        {*firstDerivationFolderName,
         {{{"../Common"}}},
         SerializationFormat::YAML,
         {
             {firstDerivationObjectName, {baseObjectName, firstDerivationObjectChangelist}},
         }},
        {*secondDerivationFolderName,
         {{{"../../Units"}}},
         SerializationFormat::YAML,
         {
             {secondDerivationObjectName, {firstDerivationObjectName, secondDerivationObjectChangelist}},
         }},
    });

    LibraryLoader loader {GetTypeManifest ()};
    loader.Begin ({{EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", secondDerivationFolderName), {}}});

    while (loader.IsLoading ())
    {
        std::this_thread::yield ();
    }

    Library library = loader.End ();
    const Library::ObjectData *firstDerivationObjectData = library.Find (firstDerivationObjectName);
    const Library::ObjectData *secondDerivationObjectData = library.Find (secondDerivationObjectName);

    REQUIRE (firstDerivationObjectData);
    REQUIRE (secondDerivationObjectData);
    CHECK_EQUAL (library.GetRegisteredObjectMap ().size (), 3u);

    CHECK_EQUAL (firstDerivationObjectData->object.parent, baseObjectName);
    ObjectComponent firstDerivationComponent {baseObjectChangelist.front ().component +
                                              firstDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (firstDerivationObjectData->object.changelist,
                             {firstDerivationComponent, baseObjectChangelist[1u], firstDerivationObjectChangelist[1u]});
    CHECK_EQUAL (firstDerivationObjectData->loadedAsParent, true);

    CHECK_EQUAL (secondDerivationObjectData->object.parent, firstDerivationObjectName);
    ObjectComponent secondDerivationComponent {firstDerivationComponent.component +
                                               secondDerivationObjectChangelist.front ().component};
    CheckChangelistEquality (
        secondDerivationObjectData->object.changelist,
        {secondDerivationComponent, baseObjectChangelist[1u], firstDerivationObjectChangelist[1u]});
    CHECK_EQUAL (secondDerivationObjectData->loadedAsParent, false);
}

END_SUITE
