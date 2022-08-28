#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Asset/Object/LibraryLoader.hpp>
#include <Asset/Object/Test/Helpers.hpp>
#include <Asset/Object/Test/Types.hpp>

#include <Container/String.hpp>
#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Asset::Object::Test
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
    Declaration declaration;
    Container::Vector<StandardLayout::Patch> localChangelist;
    SerializationFormat format = SerializationFormat::YAML;
};

struct FolderDefinition final
{
    /// \details Relative to ::ENVIRONMENT_ROOT.
    Container::String relativePath;

    /// \details Relative to this folder path.
    Container::Vector<Container::String> relativeDependencyFolders;

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

    Emergence::Serialization::Binary::PatchBundleSerializer binarySerializer;
    Emergence::Serialization::Yaml::PatchBundleSerializer yamlSerializer;

    for (const FolderDefinition &folder : _folders)
    {
        const std::filesystem::path folderPath = rootPath / folder.relativePath;
        std::filesystem::create_directories (folderPath);

        if (!folder.relativeDependencyFolders.empty ())
        {
            FolderDependency dependency;
            switch (folder.dependencySerializationFormat)
            {
            case SerializationFormat::BINARY:
            {
                std::ofstream output {folderPath / LibraryLoader::BINARY_FOLDER_DEPENDENCY_LIST, std::ios::binary};
                for (const Container::String &relativePath : folder.relativeDependencyFolders)
                {
                    REQUIRE (relativePath.size () < FolderDependency::RELATIVE_PATH_MAX_LENGTH);
                    strcpy (dependency.relativePath.data (), relativePath.c_str ());
                    Serialization::Binary::SerializeObject (output, &dependency, FolderDependency::Reflect ().mapping);
                }

                break;
            }
            case SerializationFormat::YAML:
            {
                std::ofstream output {folderPath / LibraryLoader::YAML_FOLDER_DEPENDENCY_LIST};
                Serialization::Yaml::ObjectBundleSerializer serializer {FolderDependency::Reflect ().mapping};
                serializer.Begin ();

                for (const Container::String &relativePath : folder.relativeDependencyFolders)
                {
                    REQUIRE (relativePath.size () < FolderDependency::RELATIVE_PATH_MAX_LENGTH);
                    strcpy (dependency.relativePath.data (), relativePath.c_str ());
                    serializer.Next (&dependency);
                }

                serializer.End (output);
                break;
            }
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
                std::ofstream declarationOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::BINARY_OBJECT_DECLARATION_SUFFIX)
                        .c_str (),
                    std::ios::binary};
                Serialization::Binary::SerializeObject (declarationOutput, &object.declaration,
                                                        Declaration::Reflect ().mapping);

                std::ofstream bodyOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::BINARY_OBJECT_BODY_SUFFIX)
                        .c_str (),
                    std::ios::binary};

                binarySerializer.Begin (bodyOutput);
                for (const StandardLayout::Patch &patch : object.localChangelist)
                {
                    binarySerializer.Next (patch);
                }

                binarySerializer.End ();
                break;
            }

            case SerializationFormat::YAML:
            {
                std::ofstream declarationOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::YAML_OBJECT_DECLARATION_SUFFIX)
                        .c_str ()};

                Serialization::Yaml::SerializeObject (declarationOutput, &object.declaration,
                                                      Declaration::Reflect ().mapping);

                std::ofstream bodyOutput {
                    (objectPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> () +
                     LibraryLoader::YAML_OBJECT_BODY_SUFFIX)
                        .c_str ()};

                yamlSerializer.Begin ();
                for (const StandardLayout::Patch &patch : object.localChangelist)
                {
                    yamlSerializer.Next (patch);
                }

                yamlSerializer.End (bodyOutput);
                break;
            }
            }
        }
    }
}
} // namespace Emergence::Asset::Object::Test

using namespace Emergence::Asset::Object::Test;
using namespace Emergence::Asset::Object;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (LibraryLoader)

TEST_CASE (LoadTrivial)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstObjectChangelist {
        MakePatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::YAML,
                          {
                              {firstObjectName, {}, firstObjectChangelist},
                              {secondObjectName, {}, secondObjectChangelist},
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

    CHECK_EQUAL (firstObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (firstObjectData->body.fullChangelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsDependency, false);

    CHECK_EQUAL (secondObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (secondObjectData->body.fullChangelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsDependency, false);
}

TEST_CASE (LoadSpecified)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstObjectChangelist {
        MakePatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::YAML,
                          {
                              {firstObjectName, {}, firstObjectChangelist},
                              {secondObjectName, {}, secondObjectChangelist},
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

    CHECK_EQUAL (secondObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (secondObjectData->body.fullChangelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsDependency, false);
}

TEST_CASE (LoadTrivialBinary)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"First"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstObjectChangelist {
        MakePatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Second"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::BINARY,
                          {
                              {firstObjectName, {}, firstObjectChangelist, SerializationFormat::BINARY},
                              {secondObjectName, {}, secondObjectChangelist, SerializationFormat::BINARY},
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

    CHECK_EQUAL (firstObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (firstObjectData->body.fullChangelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsDependency, false);

    CHECK_EQUAL (secondObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (secondObjectData->body.fullChangelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsDependency, false);
}

TEST_CASE (LoadTrivialSubdirectories)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString firstObjectName {"Subdir1/First"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstObjectChangelist {
        MakePatch (FirstComponent {0u, 1.0f, 2.0f, 3.0f, 4.0f})};

    const Emergence::Memory::UniqueString secondObjectName {"Subdir2/Second"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    PrepareEnvironment ({{*folderName,
                          {},
                          SerializationFormat::YAML,
                          {
                              {firstObjectName, {}, firstObjectChangelist},
                              {secondObjectName, {}, secondObjectChangelist},
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

    CHECK_EQUAL (firstObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (firstObjectData->body.fullChangelist, firstObjectChangelist);
    CHECK_EQUAL (firstObjectData->loadedAsDependency, false);

    CHECK_EQUAL (secondObjectData->declaration.parent, ""_us);
    CheckChangelistEquality (secondObjectData->body.fullChangelist, secondObjectChangelist);
    CHECK_EQUAL (secondObjectData->loadedAsDependency, false);
}

TEST_CASE (LoadInheritance)
{
    const Emergence::Memory::UniqueString folderName {"Objects"};

    const Emergence::Memory::UniqueString baseObjectName {"Base"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> baseObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    const Emergence::Memory::UniqueString firstDerivationObjectName {"FirstDerivation"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstDerivationObjectChangelist {
        MakePatch (SecondComponent {0u, 0u, 30u, 0u})};

    const Emergence::Memory::UniqueString secondDerivationObjectName {"SecondDerivation"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondDerivationObjectChangelist {
        MakePatch (SecondComponent {0u, 0u, 0u, 11u})};

    PrepareEnvironment (
        {{*folderName,
          {},
          SerializationFormat::YAML,
          {
              {baseObjectName, {}, baseObjectChangelist},
              {firstDerivationObjectName, {baseObjectName}, firstDerivationObjectChangelist},
              {secondDerivationObjectName, {firstDerivationObjectName}, secondDerivationObjectChangelist},
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

    CHECK_EQUAL (firstDerivationObjectData->declaration.parent, baseObjectName);
    Emergence::StandardLayout::Patch firstDerivationPatch =
        baseObjectChangelist.front () + firstDerivationObjectChangelist.front ();
    CheckChangelistEquality (firstDerivationObjectData->body.fullChangelist, {firstDerivationPatch});
    CHECK_EQUAL (firstDerivationObjectData->loadedAsDependency, false);

    CHECK_EQUAL (secondDerivationObjectData->declaration.parent, firstDerivationObjectName);
    Emergence::StandardLayout::Patch secondDerivationPatch =
        firstDerivationPatch + secondDerivationObjectChangelist.front ();
    CheckChangelistEquality (secondDerivationObjectData->body.fullChangelist, {secondDerivationPatch});
    CHECK_EQUAL (secondDerivationObjectData->loadedAsDependency, false);
}

TEST_CASE (LoadDependencies)
{
    const Emergence::Memory::UniqueString baseFolderName {"Objects/Common"};
    const Emergence::Memory::UniqueString firstDerivationFolderName {"Objects/Units"};
    const Emergence::Memory::UniqueString secondDerivationFolderName {"Objects/Maps/Ranglor"};

    const Emergence::Memory::UniqueString baseObjectName {"Base"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> baseObjectChangelist {
        MakePatch (SecondComponent {0u, 100u, 20u, 10u})};

    const Emergence::Memory::UniqueString firstDerivationObjectName {"FirstDerivation"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> firstDerivationObjectChangelist {
        MakePatch (SecondComponent {0u, 0u, 30u, 0u})};

    const Emergence::Memory::UniqueString secondDerivationObjectName {"SecondDerivation"};
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> secondDerivationObjectChangelist {
        MakePatch (SecondComponent {0u, 0u, 0u, 11u})};

    PrepareEnvironment ({
        {*baseFolderName,
         {},
         SerializationFormat::YAML,
         {
             {baseObjectName, {}, baseObjectChangelist},
         }},
        {*firstDerivationFolderName,
         {"../Common"},
         SerializationFormat::YAML,
         {
             {firstDerivationObjectName, {baseObjectName}, firstDerivationObjectChangelist},
         }},
        {*secondDerivationFolderName,
         {"../../Units"},
         SerializationFormat::YAML,
         {
             {secondDerivationObjectName, {firstDerivationObjectName}, secondDerivationObjectChangelist},
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

    CHECK_EQUAL (firstDerivationObjectData->declaration.parent, baseObjectName);
    Emergence::StandardLayout::Patch firstDerivationPatch =
        baseObjectChangelist.front () + firstDerivationObjectChangelist.front ();
    CheckChangelistEquality (firstDerivationObjectData->body.fullChangelist, {firstDerivationPatch});
    CHECK_EQUAL (firstDerivationObjectData->loadedAsDependency, true);

    CHECK_EQUAL (secondDerivationObjectData->declaration.parent, firstDerivationObjectName);
    Emergence::StandardLayout::Patch secondDerivationPatch =
        firstDerivationPatch + secondDerivationObjectChangelist.front ();
    CheckChangelistEquality (secondDerivationObjectData->body.fullChangelist, {secondDerivationPatch});
    CHECK_EQUAL (secondDerivationObjectData->loadedAsDependency, false);
}

END_SUITE