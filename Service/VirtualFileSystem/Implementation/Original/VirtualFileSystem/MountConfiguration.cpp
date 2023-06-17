#include <StandardLayout/MappingRegistration.hpp>

#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::VirtualFileSystem
{
const MountConfiguration::Reflection &MountConfiguration::Reflect () noexcept
{
    static const Reflection reflection = []()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MountConfiguration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (source);
        EMERGENCE_MAPPING_REGISTER_REGULAR (sourcePath);
        EMERGENCE_MAPPING_REGISTER_REGULAR (targetPath);
        EMERGENCE_MAPPING_REGISTRATION_END();
    }();

    return reflection;
}

const MountConfigurationList::Reflection &MountConfigurationList::Reflect () noexcept
{
    static const Reflection reflection = []()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MountConfigurationList);
        EMERGENCE_MAPPING_REGISTER_REGULAR (items);
        EMERGENCE_MAPPING_REGISTRATION_END();
    }();

    return reflection;
}
} // namespace Emergence::VirtualFileSystem
