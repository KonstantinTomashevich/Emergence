#include <sstream>

#include <Container/InplaceVector.hpp>

#include <Serialization/Test/Tests.hpp>
#include <Serialization/Test/Types.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Serialization::Yaml::Test
{
template <typename Type>
void ObjectSerializationDeserializationTest (const Type &_value)
{
    std::stringstream buffer;
    SerializeObject (buffer, &_value, Type::Reflect ().mapping);

    FieldNameLookupCache cache {Type::Reflect ().mapping};
    Type deserialized;

    CHECK (DeserializeObject (buffer, &deserialized, Type::Reflect ().mapping, cache));
    CHECK_EQUAL (_value, deserialized);
}
} // namespace Emergence::Serialization::Yaml::Test

using namespace Emergence::Serialization::Test;
using namespace Emergence::Serialization::Yaml::Test;

BEGIN_SUITE (YamlObjectSerialization)

OBJECT_SERIALIZATION_TESTS (ObjectSerializationDeserializationTest)

END_SUITE

#undef PATCH_TESTS
