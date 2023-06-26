#include <fstream>

#include <CommandLine/ParseArguments.hpp>

#include <Configuration/ResourceProviderTypes.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Context.hpp>
#include <Resource/Cooking/Pass/AllResourceFlatIndex.hpp>
#include <Resource/Cooking/Pass/AllResourceImport.hpp>
#include <Resource/Cooking/Pass/BinaryConversion.hpp>
#include <Resource/Cooking/Result/FlatPackage.hpp>
#include <Resource/Cooking/Result/MountList.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <VirtualFileSystem/Context.hpp>

enum class ExitCode : int
{
    SUCCESSFUL = 0,
    INVALID_ARGUMENTS,
    UNKNOWN_MOUNT_LIST_FORMAT,
    FAILED_TO_OPEN_MOUNT_LIST,
    FAILED_TO_DESERIALIZE_MOUNT_LIST,
    FAILED_TO_SETUP_COOKING_CONTEXT,
    FAILED_ALL_RESOURCE_IMPORT_PASS,
    FAILED_BINARY_CONVERSION_PASS,
    FAILED_ALL_RESOURCE_FLAT_INDEX_PASS,
    FAILED_TO_PRODUCE_MOUNT_LIST,
    FAILED_TO_PRODUCE_FLAT_PACKAGE,
};

struct Arguments final
{
    Emergence::Container::Utf8String resultGroupName;
    Emergence::Container::Utf8String mountList;
    Emergence::Container::Utf8String workspace;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId resultGroupName;
        Emergence::StandardLayout::FieldId mountList;
        Emergence::StandardLayout::FieldId workspace;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const Arguments::Reflection &Arguments::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Arguments);
        EMERGENCE_MAPPING_REGISTER_REGULAR (resultGroupName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (mountList);
        EMERGENCE_MAPPING_REGISTER_REGULAR (workspace);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

int main (int _argumentCount, char **_arguments)
{
    Arguments arguments;
    if (!Emergence::CommandLine::ParseArguments (_argumentCount, _arguments, &arguments, Arguments::Reflect ().mapping))
    {
        return static_cast<int> (ExitCode::INVALID_ARGUMENTS);
    }

    if (arguments.resultGroupName.empty () || arguments.mountList.empty () || arguments.workspace.empty ())
    {
        return static_cast<int> (ExitCode::INVALID_ARGUMENTS);
    }

    bool binaryMountList;
    if (arguments.mountList.ends_with ("bin"))
    {
        binaryMountList = true;
    }
    else if (arguments.mountList.ends_with ("yaml"))
    {
        binaryMountList = false;
    }
    else
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to determine mount list format.");
        return static_cast<int> (ExitCode::UNKNOWN_MOUNT_LIST_FORMAT);
    }

    std::ifstream mountListInput {arguments.mountList.c_str (), binaryMountList ? std::ios::binary : std::ios::in};
    if (!mountListInput)
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to open mount list file \"", arguments.mountList, "\".");
        return static_cast<int> (ExitCode::FAILED_TO_OPEN_MOUNT_LIST);
    }

    Emergence::VirtualFileSystem::MountConfigurationList mountList;
    if (binaryMountList)
    {
        if (!Emergence::Serialization::Binary::DeserializeObject (
                mountListInput, &mountList, Emergence::VirtualFileSystem::MountConfigurationList::Reflect ().mapping,
                {}))
        {
            EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to deserialize mount list \"", arguments.mountList, "\".");
            return static_cast<int> (ExitCode::FAILED_TO_DESERIALIZE_MOUNT_LIST);
        }
    }
    else
    {
        if (!Emergence::Serialization::Yaml::DeserializeObject (
                mountListInput, &mountList, Emergence::VirtualFileSystem::MountConfigurationList::Reflect ().mapping,
                {}))
        {
            EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to deserialize mount list \"", arguments.mountList, "\".");
            return static_cast<int> (ExitCode::FAILED_TO_DESERIALIZE_MOUNT_LIST);
        }
    }

    Emergence::Resource::Cooking::Context context {GetResourceTypesRegistry (), GetPatchableTypesRegistry ()};
    if (!context.Setup (mountList, arguments.workspace))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to setup cooking context.");
        return static_cast<int> (ExitCode::FAILED_TO_SETUP_COOKING_CONTEXT);
    }

    if (!Emergence::Resource::Cooking::AllResourceImportPass (context))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed all resource import pass.");
        return static_cast<int> (ExitCode::FAILED_ALL_RESOURCE_IMPORT_PASS);
    }

    if (!Emergence::Resource::Cooking::BinaryConversionPass (context))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed binary conversion pass.");
        return static_cast<int> (ExitCode::FAILED_BINARY_CONVERSION_PASS);
    }

    if (!Emergence::Resource::Cooking::AllResourceFlatIndexPass (context))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed all resource flat index pass.");
        return static_cast<int> (ExitCode::FAILED_ALL_RESOURCE_FLAT_INDEX_PASS);
    }

    const Emergence::Container::Utf8String packageName = EMERGENCE_BUILD_STRING (arguments.resultGroupName, ".pack");
    if (!Emergence::Resource::Cooking::ProduceFlatPackage (context, packageName))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed to produce flat package.");
        return static_cast<int> (ExitCode::FAILED_TO_PRODUCE_FLAT_PACKAGE);
    }

    Emergence::VirtualFileSystem::MountConfigurationList resultMountList;
    resultMountList.items.emplace_back () = {Emergence::VirtualFileSystem::MountSource::PACKAGE, packageName,
                                             arguments.resultGroupName};

    if (!Emergence::Resource::Cooking::ProduceMountList (context, arguments.resultGroupName, resultMountList))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed to produce mount list.");
        return static_cast<int> (ExitCode::FAILED_TO_PRODUCE_MOUNT_LIST);
    }

    return static_cast<int> (ExitCode::SUCCESSFUL);
}
