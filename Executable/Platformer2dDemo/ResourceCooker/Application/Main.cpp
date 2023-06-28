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

#include <StandardLayout/MappingRegistration.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/Helpers.hpp>

enum class ExitCode : int
{
    SUCCESSFUL = 0,
    INVALID_ARGUMENTS,
    FAILED_TO_FETCH_MOUNT_LIST,
    FAILED_TO_SETUP_COOKING_CONTEXT,
    FAILED_ALL_RESOURCE_IMPORT_PASS,
    FAILED_BINARY_CONVERSION_PASS,
    FAILED_ALL_RESOURCE_FLAT_INDEX_PASS,
    FAILED_TO_PRODUCE_MOUNT_LIST,
    FAILED_TO_PRODUCE_FLAT_PACKAGE,
};

struct Arguments final
{
    Emergence::Container::Utf8String groupName;
    Emergence::Container::Utf8String mountListDirectory;
    Emergence::Container::Utf8String workspace;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId groupName;
        Emergence::StandardLayout::FieldId mountListDirectory;
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
        EMERGENCE_MAPPING_REGISTER_REGULAR (groupName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (mountListDirectory);
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

    if (arguments.groupName.empty () || arguments.mountListDirectory.empty () || arguments.workspace.empty ())
    {
        return static_cast<int> (ExitCode::INVALID_ARGUMENTS);
    }

    Emergence::VirtualFileSystem::MountConfigurationList mountList;
    if (!Emergence::VirtualFileSystem::FetchMountConfigurationList (arguments.mountListDirectory, arguments.groupName,
                                                                    mountList))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Unable to fetch mount list for group \"", arguments.groupName,
                       "\" in directory \"", arguments.mountListDirectory, "\".");
        return static_cast<int> (ExitCode::FAILED_TO_FETCH_MOUNT_LIST);
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

    const Emergence::Container::Utf8String packageName = EMERGENCE_BUILD_STRING (arguments.groupName, ".pack");
    if (!Emergence::Resource::Cooking::ProduceFlatPackage (context, packageName))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed to produce flat package.");
        return static_cast<int> (ExitCode::FAILED_TO_PRODUCE_FLAT_PACKAGE);
    }

    Emergence::VirtualFileSystem::MountConfigurationList resultMountList;
    resultMountList.items.emplace_back () = {Emergence::VirtualFileSystem::MountSource::PACKAGE, packageName,
                                             arguments.groupName};

    if (!Emergence::Resource::Cooking::ProduceMountList (context, arguments.groupName, resultMountList))
    {
        EMERGENCE_LOG (ERROR, "ResourceCooker: Failed to produce mount list.");
        return static_cast<int> (ExitCode::FAILED_TO_PRODUCE_MOUNT_LIST);
    }

    return static_cast<int> (ExitCode::SUCCESSFUL);
}
