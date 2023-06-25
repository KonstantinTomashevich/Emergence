#include <filesystem>
#include <fstream>

#include <Resource/Cooking/Test/Environment.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Resource::Cooking::Test
{
static const char *ENVIRONMENT_ROOT = "Environment";
static const char *INPUT_ROOT = "Input";
static const char *WORKSPACE_ROOT = "Workspace";

const FirstObjectType::Reflection &FirstObjectType::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FirstObjectType);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const SecondObjectType::Reflection &SecondObjectType::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SecondObjectType);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (cost);
        EMERGENCE_MAPPING_REGISTER_REGULAR (time);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Container::MappingRegistry &GetResourceObjectTypes () noexcept
{
    static const Container::MappingRegistry registry = [] ()
    {
        Container::MappingRegistry types;
        types.Register (FirstObjectType::Reflect ().mapping);
        types.Register (SecondObjectType::Reflect ().mapping);
        return types;
    }();

    return registry;
}

void PrepareEnvironmentAndSetupContext (Context &_context, const Environment &_environment)
{
    if (std::filesystem::exists (ENVIRONMENT_ROOT))
    {
        REQUIRE (std::filesystem::remove_all (ENVIRONMENT_ROOT));
    }

    const Container::Utf8String inputRoot =
        EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, VirtualFileSystem::PATH_SEPARATOR, INPUT_ROOT);

    const Container::Utf8String workspaceRoot =
        EMERGENCE_BUILD_STRING (ENVIRONMENT_ROOT, VirtualFileSystem::PATH_SEPARATOR, WORKSPACE_ROOT);

    std::filesystem::create_directories (inputRoot);
    auto serializeObject = [&inputRoot] (const auto &_resourceObject)
    {
        using Type = typename std::decay_t<decltype (_resourceObject)>::DataType;
        const Container::Utf8String fullPath =
            EMERGENCE_BUILD_STRING (inputRoot, VirtualFileSystem::PATH_SEPARATOR, _resourceObject.path);
        std::filesystem::create_directories (std::filesystem::path {fullPath}.parent_path ());

        if (fullPath.ends_with ("yaml"))
        {
            std::ofstream output {fullPath.c_str ()};
            REQUIRE (output);
            Serialization::Yaml::SerializeTypeName (output, Type::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (output, &_resourceObject.data, Type::Reflect ().mapping);
        }
        else if (fullPath.ends_with ("bin"))
        {
            std::ofstream output {fullPath.c_str (), std::ios::binary};
            REQUIRE (output);
            Serialization::Binary::SerializeTypeName (output, Type::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (output, &_resourceObject.data, Type::Reflect ().mapping);
        }
        else
        {
            REQUIRE (false);
        }
    };

    for (const ResourceInfo<FirstObjectType> &object : _environment.objectsFirst)
    {
        serializeObject (object);
    }

    for (const ResourceInfo<SecondObjectType> &object : _environment.objectsSecond)
    {
        serializeObject (object);
    }

    for (const ResourceInfo<Container::Vector<std::uint8_t>> &resource : _environment.thirdParty)
    {
        const Container::Utf8String fullPath =
            EMERGENCE_BUILD_STRING (inputRoot, VirtualFileSystem::PATH_SEPARATOR, resource.path);
        std::filesystem::create_directories (std::filesystem::path {fullPath}.parent_path ());

        std::ofstream output {fullPath.c_str (), std::ios::binary};
        REQUIRE (output);
        output.write (reinterpret_cast<const char *> (resource.data.data ()),
                      static_cast<std::streamsize> (resource.data.size ()));
    }

    REQUIRE (
        _context.Setup ({{{VirtualFileSystem::MountSource::FILE_SYSTEM, inputRoot, "TestResources"}}}, workspaceRoot));
}

Container::Utf8String GetFinalResultRealPath (const Context &_context, const std::string_view &_resultName) noexcept
{
    return EMERGENCE_BUILD_STRING (
        ENVIRONMENT_ROOT, VirtualFileSystem::PATH_SEPARATOR, WORKSPACE_ROOT, VirtualFileSystem::PATH_SEPARATOR,
        _context.GetFinalResultDirectory ().GetFullFileName (), VirtualFileSystem::PATH_SEPARATOR, _resultName);
}
} // namespace Emergence::Resource::Cooking::Test
