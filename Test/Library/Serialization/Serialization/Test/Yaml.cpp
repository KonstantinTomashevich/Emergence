#include <sstream>

#include <Container/InplaceVector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

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

void PatchBundleSerializeAndDeserialize (const Container::Vector<StandardLayout::Patch> &_patchesToSerialize,
                                         Container::Vector<StandardLayout::Patch> &_deserializationOutput)
{
    using namespace Emergence::Serialization::Test;
    std::stringstream buffer;
    PatchBundleSerializer serializer;

    serializer.Begin ();
    for (const StandardLayout::Patch &patch : _patchesToSerialize)
    {
        serializer.Next (patch);
    }

    serializer.End (buffer);

    PatchBundleDeserializer deserializer;
    deserializer.RegisterType (TrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (NonTrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (UnionStruct::Reflect ().mapping);

    REQUIRE (deserializer.Begin (buffer));
    while (deserializer.HasNext ())
    {
        Emergence::Container::Optional<Emergence::StandardLayout::Patch> result = deserializer.Next ();
        REQUIRE (result.has_value ());
        _deserializationOutput.emplace_back (result.value ());
    }

    deserializer.End ();
}

namespace ObjectBundle
{
using namespace Emergence::Memory::Literals;
using namespace Emergence::Serialization::Test;

static const std::array<NonTrivialStruct, 3u> ITEMS {
    NonTrivialStruct {0b111u, {"Hello, world!"}, "For gold and glory!"_us},
    NonTrivialStruct {0b001u, {"Second!"}, "For the House!"_us},
    NonTrivialStruct {0b100u, {"Third!"}, "For the golden dragon!"_us}};

static const char *YAML =
    "- alive: true\n"
    "  poisoned: true\n"
    "  stunned: true\n"
    "  string: Hello, world!\n"
    "  uniqueString: For gold and glory!\n"
    "- alive: true\n"
    "  poisoned: false\n"
    "  stunned: false\n"
    "  string: Second!\n"
    "  uniqueString: For the House!\n"
    "- alive: false\n"
    "  poisoned: false\n"
    "  stunned: true\n"
    "  string: Third!\n"
    "  uniqueString: For the golden dragon!";
} // namespace ObjectBundle
} // namespace Emergence::Serialization::Yaml::Test

using namespace Emergence::Serialization::Test;
using namespace Emergence::Serialization::Yaml::Test;
using namespace Emergence::Serialization::Yaml;

BEGIN_SUITE (YamlObjectSerialization)

OBJECT_SERIALIZATION_TESTS (ObjectSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (YamlPatchSerialization)

PATCH_SERIALIZATION_TESTS (PatchSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (YamlObjectBundleSerialization)

// TODO: Test reusability?

TEST_CASE (MultipleNonTrivial)
{
    std::stringstream buffer;
    ObjectBundleSerializer serializer {NonTrivialStruct::Reflect ().mapping};

    serializer.Begin ();
    for (const NonTrivialStruct &item : ObjectBundle::ITEMS)
    {
        serializer.Next (&item);
    }

    serializer.End (buffer);
    CHECK_EQUAL (buffer.str (), ObjectBundle::YAML);
}

END_SUITE

BEGIN_SUITE (YamlObjectBundleDeserialization)

// TODO: Test reusability?

TEST_CASE (MultipleNonTrivial)
{
    std::stringstream buffer;
    std::decay_t<decltype (ObjectBundle::ITEMS)> items;

    buffer << ObjectBundle::YAML;
    ObjectBundleDeserializer deserializer {NonTrivialStruct::Reflect ().mapping};

    deserializer.Begin (buffer);
    for (NonTrivialStruct &item : items)
    {
        REQUIRE (deserializer.HasNext ());
        deserializer.Next (&item);
    }

    CHECK (!deserializer.HasNext ());
    deserializer.End ();
    CHECK_EQUAL (items, ObjectBundle::ITEMS);
}

END_SUITE

BEGIN_SUITE (YamlPatchBundleSerialization)

PATCH_BUNDLE_SERIALIZATION_TESTS (PatchBundleSerializeAndDeserialize)

END_SUITE
