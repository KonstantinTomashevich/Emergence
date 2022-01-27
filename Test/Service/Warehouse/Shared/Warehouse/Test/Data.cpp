#include <StandardLayout/MappingRegistration.hpp>

#include <Warehouse/Test/Data.hpp>

namespace Emergence::Warehouse::Test
{
const TestSingleton::Reflection &TestSingleton::Reflect ()
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (data)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

const TestSingleton TestSingleton::NON_DEFAULT_INSTANCE {.data = {0u, 1u, 2u, 3u}};
} // namespace Emergence::Warehouse::Test
