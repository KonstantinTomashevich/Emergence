#include <ResourceProvider/IndexFile.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::ResourceProvider
{
const IndexFileObjectItem::Reflection &IndexFileObjectItem::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (IndexFileObjectItem);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (typeName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const IndexFileThirdPartyItem::Reflection &IndexFileThirdPartyItem::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (IndexFileThirdPartyItem);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const IndexFile::Reflection &IndexFile::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (IndexFile);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objects);
        EMERGENCE_MAPPING_REGISTER_REGULAR (thirdParty);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::ResourceProvider
