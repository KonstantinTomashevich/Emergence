#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <filesystem>
#include <fstream>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Provider/IndexFile.hpp>
#include <Resource/Provider/ResourceProvider.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Resource::Provider::Test
{
namespace
{
const char *const ENVIRONMENT_ROOT = "./Environment";
const char *const ENVIRONMENT_MOUNT = "Environment";

struct TestResourceObjectFirst final
{
    Memory::UniqueString prefabId;
    float x = 0.0f;
    float y = 0.0f;

    bool operator== (const TestResourceObjectFirst &_other) const noexcept = default;

    bool operator!= (const TestResourceObjectFirst &_other) const noexcept = default;

    struct Reflection final
    {
        StandardLayout::FieldId prefabId;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TestResourceObjectFirst::Reflection &TestResourceObjectFirst::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestResourceObjectFirst);
        EMERGENCE_MAPPING_REGISTER_REGULAR (prefabId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct TestResourceObjectSecond final
{
    Memory::UniqueString configA;
    Memory::UniqueString configB;
    Memory::UniqueString configC;

    bool operator== (const TestResourceObjectSecond &_other) const noexcept = default;

    bool operator!= (const TestResourceObjectSecond &_other) const noexcept = default;

    struct Reflection final
    {
        StandardLayout::FieldId configA;
        StandardLayout::FieldId configB;
        StandardLayout::FieldId configC;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TestResourceObjectSecond::Reflection &TestResourceObjectSecond::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestResourceObjectSecond);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configA);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configB);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configC);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

template <typename Type>
struct IdentifiedObject final
{
    Memory::UniqueString id;
    Type object;

    bool operator== (const IdentifiedObject<Type> &_other) const noexcept = default;

    bool operator!= (const IdentifiedObject<Type> &_other) const noexcept = default;
};

template <typename Type>
struct ResourceObject final
{
    Type object;
    Container::String relativePath;
};

struct ResourceSourceDescription final
{
    Memory::UniqueString path;
    bool index = false;

    Container::Vector<ResourceObject<TestResourceObjectFirst>> firstObjectBinary;
    Container::Vector<ResourceObject<TestResourceObjectFirst>> firstObjectYaml;

    Container::Vector<ResourceObject<TestResourceObjectSecond>> secondObjectBinary;
    Container::Vector<ResourceObject<TestResourceObjectSecond>> secondObjectYaml;

    Container::Vector<ResourceObject<Container::Vector<std::uint8_t>>> thirdPartyResources;
};

Container::MappingRegistry GetObjectTypeRegistry ()
{
    Container::MappingRegistry registry;
    registry.Register (TestResourceObjectFirst::Reflect ().mapping);
    registry.Register (TestResourceObjectSecond::Reflect ().mapping);
    return registry;
}

VirtualFileSystem::Context SetupEnvironment (const Container::Vector<ResourceSourceDescription> &_sources)
{
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};
    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    std::filesystem::create_directories (rootPath);
    Emergence::VirtualFileSystem::Context virtualFileSystem;
    REQUIRE (virtualFileSystem.Mount (virtualFileSystem.GetRoot (), {VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                     ENVIRONMENT_ROOT, ENVIRONMENT_MOUNT}));

    for (const ResourceSourceDescription &source : _sources)
    {
        const std::filesystem::path sourcePath = rootPath / *source.path;
        std::filesystem::create_directories (sourcePath);

        for (const ResourceObject<TestResourceObjectFirst> &object : source.firstObjectBinary)
        {
            std::filesystem::create_directories (
                std::filesystem::path {sourcePath / object.relativePath}.parent_path ());
            std::ofstream output (sourcePath / object.relativePath, std::ios::binary);
            Serialization::Binary::SerializeTypeName (output, TestResourceObjectFirst::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (output, &object.object,
                                                    TestResourceObjectFirst::Reflect ().mapping);
        }

        for (const ResourceObject<TestResourceObjectFirst> &object : source.firstObjectYaml)
        {
            std::filesystem::create_directories (
                std::filesystem::path {sourcePath / object.relativePath}.parent_path ());
            std::ofstream output (sourcePath / object.relativePath);
            Serialization::Yaml::SerializeTypeName (output, TestResourceObjectFirst::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (output, &object.object, TestResourceObjectFirst::Reflect ().mapping);
        }

        for (const ResourceObject<TestResourceObjectSecond> &object : source.secondObjectBinary)
        {
            std::filesystem::create_directories (
                std::filesystem::path {sourcePath / object.relativePath}.parent_path ());
            std::ofstream output (sourcePath / object.relativePath, std::ios::binary);
            Serialization::Binary::SerializeTypeName (output, TestResourceObjectSecond::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (output, &object.object,
                                                    TestResourceObjectSecond::Reflect ().mapping);
        }

        for (const ResourceObject<TestResourceObjectSecond> &object : source.secondObjectYaml)
        {
            std::filesystem::create_directories (
                std::filesystem::path {sourcePath / object.relativePath}.parent_path ());
            std::ofstream output (sourcePath / object.relativePath);
            Serialization::Yaml::SerializeTypeName (output, TestResourceObjectSecond::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (output, &object.object, TestResourceObjectSecond::Reflect ().mapping);
        }

        for (const ResourceObject<Container::Vector<std::uint8_t>> &resource : source.thirdPartyResources)
        {
            std::filesystem::create_directories (
                std::filesystem::path {sourcePath / resource.relativePath}.parent_path ());
            std::ofstream output (sourcePath / resource.relativePath, std::ios::binary);
            output.write (reinterpret_cast<const char *> (resource.object.data ()),
                          static_cast<std::streamsize> (resource.object.size ()));
        }

        if (source.index)
        {
            ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
            Container::Utf8String sourcePathString {
                sourcePath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};

            // Fallback behaviour to replace Windows separators with VFS ones if necessary.
            std::replace (sourcePathString.begin (), sourcePathString.end (), '\\', VirtualFileSystem::PATH_SEPARATOR);

            const Memory::UniqueString sourcePathUniqueString {sourcePathString.c_str ()};
            REQUIRE (provider.AddSource (sourcePathUniqueString) == SourceOperationResponse::SUCCESSFUL);

            VirtualFileSystem::Entry indexOutput {virtualFileSystem.CreateFile (
                VirtualFileSystem::Entry {virtualFileSystem, sourcePathString}, IndexFile::INDEX_FILE_NAME)};

            REQUIRE (provider.SaveSourceIndex (sourcePathUniqueString, indexOutput) ==
                     SourceOperationResponse::SUCCESSFUL);
        }
    }

    return virtualFileSystem;
}

struct Expectation final
{
    Container::Vector<IdentifiedObject<TestResourceObjectFirst>> firstObjects;
    Container::Vector<IdentifiedObject<TestResourceObjectSecond>> secondObjects;
    Container::Vector<IdentifiedObject<Container::Vector<std::uint8_t>>> thirdParty;
};

void AddToExpectation (Expectation &_expectation, const ResourceSourceDescription &_source)
{
    for (const ResourceObject<TestResourceObjectFirst> &object : _source.firstObjectBinary)
    {
        _expectation.firstObjects.emplace_back (IdentifiedObject<TestResourceObjectFirst> {
            Memory::UniqueString {std::filesystem::path {object.relativePath}.stem ().string ().c_str ()},
            object.object});
    }

    for (const ResourceObject<TestResourceObjectFirst> &object : _source.firstObjectYaml)
    {
        _expectation.firstObjects.emplace_back (IdentifiedObject<TestResourceObjectFirst> {
            Memory::UniqueString {std::filesystem::path {object.relativePath}.stem ().string ().c_str ()},
            object.object});
    }

    for (const ResourceObject<TestResourceObjectSecond> &object : _source.secondObjectBinary)
    {
        _expectation.secondObjects.emplace_back (IdentifiedObject<TestResourceObjectSecond> {
            Memory::UniqueString {std::filesystem::path {object.relativePath}.stem ().string ().c_str ()},
            object.object});
    }

    for (const ResourceObject<TestResourceObjectSecond> &object : _source.secondObjectYaml)
    {
        _expectation.secondObjects.emplace_back (IdentifiedObject<TestResourceObjectSecond> {
            Memory::UniqueString {std::filesystem::path {object.relativePath}.stem ().string ().c_str ()},
            object.object});
    }

    for (const ResourceObject<Container::Vector<std::uint8_t>> &resource : _source.thirdPartyResources)
    {
        _expectation.thirdParty.emplace_back (IdentifiedObject<Container::Vector<std::uint8_t>> {
            Memory::UniqueString {std::filesystem::path {resource.relativePath}.filename ().string ().c_str ()},
            resource.object});
    }
}

void CheckExpectation (const Expectation &_expectation, const ResourceProvider &_provider)
{
    Container::Vector<IdentifiedObject<TestResourceObjectFirst>> firstObjects;
    Container::Vector<IdentifiedObject<TestResourceObjectSecond>> secondObjects;
    Container::Vector<IdentifiedObject<Container::Vector<std::uint8_t>>> thirdPartyResources;

    for (ResourceProvider::ObjectRegistryCursor cursor =
             _provider.FindObjectsByType (TestResourceObjectFirst::Reflect ().mapping);
         **cursor; ++cursor)
    {
        TestResourceObjectFirst object;
        REQUIRE (_provider.LoadObject (TestResourceObjectFirst::Reflect ().mapping, *cursor, &object) ==
                 LoadingOperationResponse::SUCCESSFUL);

        firstObjects.emplace_back (IdentifiedObject<TestResourceObjectFirst> {*cursor, object});
    }

    CHECK_EQUAL (firstObjects.size (), _expectation.firstObjects.size ());
    for (const IdentifiedObject<TestResourceObjectFirst> &object : firstObjects)
    {
        CHECK (std::find (_expectation.firstObjects.begin (), _expectation.firstObjects.end (), object) !=
               _expectation.firstObjects.end ());
    }

    for (ResourceProvider::ObjectRegistryCursor cursor =
             _provider.FindObjectsByType (TestResourceObjectSecond::Reflect ().mapping);
         **cursor; ++cursor)
    {
        TestResourceObjectSecond object;
        REQUIRE (_provider.LoadObject (TestResourceObjectSecond::Reflect ().mapping, *cursor, &object) ==
                 LoadingOperationResponse::SUCCESSFUL);

        secondObjects.emplace_back (IdentifiedObject<TestResourceObjectSecond> {*cursor, object});
    }

    CHECK_EQUAL (secondObjects.size (), _expectation.secondObjects.size ());
    for (const IdentifiedObject<TestResourceObjectSecond> &object : secondObjects)
    {
        CHECK (std::find (_expectation.secondObjects.begin (), _expectation.secondObjects.end (), object) !=
               _expectation.secondObjects.end ());
    }

    Memory::Heap thirdPartyHeap {Memory::Profiler::AllocationGroup::Top ()};
    for (ResourceProvider::ThirdPartyRegistryCursor cursor = _provider.VisitAllThirdParty (); **cursor; ++cursor)
    {
        std::uint64_t thirdPartySize = 0u;
        std::uint8_t *thirdPartyData = nullptr;

        REQUIRE (_provider.LoadThirdPartyResource (*cursor, thirdPartyHeap, thirdPartySize, thirdPartyData) ==
                 LoadingOperationResponse::SUCCESSFUL);

        Container::Vector<std::uint8_t> data;
        data.reserve (static_cast<std::size_t> (thirdPartySize));

        for (std::uint64_t index = 0u; index < thirdPartySize; ++index)
        {
            data.emplace_back (thirdPartyData[index]);
        }

        thirdPartyResources.emplace_back (
            IdentifiedObject<Container::Vector<std::uint8_t>> {*cursor, std::move (data)});
        thirdPartyHeap.Release (thirdPartyData, thirdPartySize);
    }

    CHECK_EQUAL (thirdPartyResources.size (), _expectation.thirdParty.size ());
    for (const IdentifiedObject<Container::Vector<std::uint8_t>> &resource : thirdPartyResources)
    {
        CHECK (std::find (_expectation.thirdParty.begin (), _expectation.thirdParty.end (), resource) !=
               _expectation.thirdParty.end ());
    }
}
} // namespace
} // namespace Emergence::Resource::Provider::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Provider::Test;
using namespace Emergence::Resource::Provider;

BEGIN_SUITE (SingleSource)

TEST_CASE (FirstObjectYaml)
{
    ResourceSourceDescription source {
        "Source"_us,
        false,
        {},
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.yaml"},
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.yaml"},
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.yaml"},
        },
        {},
        {},
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

TEST_CASE (FirstObjectBinary)
{
    ResourceSourceDescription source {
        "Source"_us,
        false,
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.bin"},
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.bin"},
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_MOUNT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

TEST_CASE (FirstObjectMixed)
{
    ResourceSourceDescription source {
        "Source"_us,
        false,
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.bin"},
            {{"Warrior2"_us, 6.0f, 2.0f}, "Objects/WarriorPlacement2.bin"},
        },
        {
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.yaml"},
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.yaml"},
        },
        {},
        {},
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_MOUNT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

TEST_CASE (TypesMixed)
{
    ResourceSourceDescription source {
        "Source"_us,
        false,
        {},
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.yaml"},
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.yaml"},
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.yaml"},
        },
        {
            {{"A1"_us, "B1"_us, "C1"_us}, "Configs/1.bin"},
            {{"A2"_us, "B2"_us, "C2"_us}, "Configs/2.bin"},
        },
        {
            {{"A3"_us, "B3"_us, "C3"_us}, "Configs/3.yaml"},
        },
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_MOUNT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

TEST_CASE (Indexed)
{
    ResourceSourceDescription source {
        "Source"_us,
        true,
        {},
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.yaml"},
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.yaml"},
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.yaml"},
        },
        {
            {{"A1"_us, "B1"_us, "C1"_us}, "Configs/1.bin"},
            {{"A2"_us, "B2"_us, "C2"_us}, "Configs/2.bin"},
        },
        {
            {{"A3"_us, "B3"_us, "C3"_us}, "Configs/3.yaml"},
        },
        {
            {{13u, 10u, 122u, 253u, 11u, 55u, 69u, 11u}, "Test.someformat"},
        },
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_MOUNT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

TEST_CASE (ThirdParty)
{
    ResourceSourceDescription source {
        "Source"_us,
        false,
        {},
        {},
        {},
        {},
        {
            {{13u, 10u, 122u, 253u, 11u, 55u, 69u, 11u}, "Test.someformat"},
            {{11u, 12u, 126u, 255u, 0u, 9u, 97u, 0u}, "Textures/Test.sometextureformat"},
        },
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem = SetupEnvironment ({source});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {
                 EMERGENCE_BUILD_STRING (ENVIRONMENT_MOUNT, "/", source.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, source);
    CheckExpectation (expectation, provider);
}

END_SUITE

BEGIN_SUITE (MultipleSources)

TEST_CASE (Combine)
{
    ResourceSourceDescription firstSource {
        "FirstSource"_us,
        false,
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    ResourceSourceDescription secondSource {
        "SecondSource"_us,
        false,
        {
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    ResourceSourceDescription thirdSource {
        "ThirdSource"_us,
        false,
        {
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem =
        SetupEnvironment ({firstSource, secondSource, thirdSource});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};

    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", firstSource.path)}) == SourceOperationResponse::SUCCESSFUL);
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", secondSource.path)}) == SourceOperationResponse::SUCCESSFUL);
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", thirdSource.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, firstSource);
    AddToExpectation (expectation, secondSource);
    AddToExpectation (expectation, thirdSource);
    CheckExpectation (expectation, provider);
}

TEST_CASE (Remove)
{
    ResourceSourceDescription firstSource {
        "FirstSource"_us,
        false,
        {
            {{"Warrior"_us, 5.0f, 3.0f}, "Objects/WarriorPlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    ResourceSourceDescription secondSource {
        "SecondSource"_us,
        false,
        {
            {{"Mage"_us, 9.5f, 9.5f}, "Objects/MagePlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    ResourceSourceDescription thirdSource {
        "ThirdSource"_us,
        false,
        {
            {{"Archer"_us, 11.0f, 4.0f}, "Objects/ArcherPlacement.bin"},
        },
        {},
        {},
        {},
        {},
    };

    Emergence::VirtualFileSystem::Context virtualFileSystem =
        SetupEnvironment ({firstSource, secondSource, thirdSource});
    ResourceProvider provider {&virtualFileSystem, GetObjectTypeRegistry (), {}};

    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", firstSource.path)}) == SourceOperationResponse::SUCCESSFUL);
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", secondSource.path)}) == SourceOperationResponse::SUCCESSFUL);
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", thirdSource.path)}) == SourceOperationResponse::SUCCESSFUL);
    REQUIRE (provider.RemoveSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                 ENVIRONMENT_MOUNT, "/", secondSource.path)}) == SourceOperationResponse::SUCCESSFUL);

    Expectation expectation;
    AddToExpectation (expectation, firstSource);
    AddToExpectation (expectation, thirdSource);
    CheckExpectation (expectation, provider);
}

END_SUITE
