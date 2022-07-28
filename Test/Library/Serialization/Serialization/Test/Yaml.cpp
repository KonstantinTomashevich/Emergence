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

    CHECK (DeserializeObject (buffer, &deserialized, cache));
    CHECK_EQUAL (_value, deserialized);
}

template <typename Type>
void PatchSerializationDeserializationTest (const Type &_initial, const Type &_changed)
{
    std::stringstream buffer;
    SerializePatch (buffer,
                    StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &_initial));

    StandardLayout::PatchBuilder builder;
    FieldNameLookupCache cache {Type::Reflect ().mapping};
    CHECK (DeserializePatch (buffer, builder, cache));

    Type target = _initial;
    CHECK_NOT_EQUAL (target, _changed);
    builder.End ().Apply (&target);
    CHECK_EQUAL (target, _changed);
}
} // namespace Emergence::Serialization::Yaml::Test

using namespace Emergence::Serialization::Test;
using namespace Emergence::Serialization::Yaml::Test;

BEGIN_SUITE (YamlObjectSerialization)

OBJECT_SERIALIZATION_TESTS (ObjectSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (YamlPatchSerialization)

PATCH_SERIALIZATION_TESTS (PatchSerializationDeserializationTest)

END_SUITE

#undef PATCH_TESTS
