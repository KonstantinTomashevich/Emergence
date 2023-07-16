#include <StandardLayout/MappingRegistration.hpp>

#include <VirtualFileSystem/Original/PackageFile.hpp>

namespace Emergence::VirtualFileSystem::Original
{
const PackageHeaderEntry::Reflection &PackageHeaderEntry::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PackageHeaderEntry);
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
        EMERGENCE_MAPPING_REGISTER_REGULAR (offset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const PackageHeader::Reflection &PackageHeader::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PackageHeader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (entries);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::VirtualFileSystem::Original
