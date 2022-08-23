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

TEST_CASE (DifferentTypes)
{
    const TrivialStruct trivialStructInitial;
    const NonTrivialStruct nonTrivialStructInitial;
    const UnionStruct unionStructInitial {};

    TrivialStruct trivialStructChanged;
    trivialStructChanged.int8 = -32;
    trivialStructChanged.uint32 = 1537;

    NonTrivialStruct nonTrivialStructChanged;
    nonTrivialStructChanged.flags = 1u << NonTrivialStruct::ALIVE_OFFSET;
    nonTrivialStructChanged.uniqueString = Emergence::Memory::UniqueString {"For honor!"};

    UnionStruct unionStructChanged;
    unionStructChanged.type = 1u;
    unionStructChanged.m = 4u;
    unionStructChanged.n = 17u;

    std::stringstream buffer;
    PatchBundleSerializer serializer;

    serializer.Begin ();
    serializer.Next (Emergence::StandardLayout::PatchBuilder::FromDifference (
        NonTrivialStruct::Reflect ().mapping, &nonTrivialStructChanged, &nonTrivialStructInitial));

    serializer.Next (Emergence::StandardLayout::PatchBuilder::FromDifference (
        TrivialStruct::Reflect ().mapping, &trivialStructChanged, &trivialStructInitial));

    serializer.Next (Emergence::StandardLayout::PatchBuilder::FromDifference (
        UnionStruct::Reflect ().mapping, &unionStructChanged, &unionStructInitial));
    serializer.End (buffer);

    PatchBundleDeserializer deserializer;
    deserializer.RegisterType (TrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (NonTrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (UnionStruct::Reflect ().mapping);

    REQUIRE (deserializer.Begin (buffer));
    Emergence::Container::Vector<Emergence::StandardLayout::Patch> patches;

    while (deserializer.HasNext ())
    {
        Emergence::Container::Optional<Emergence::StandardLayout::Patch> result = deserializer.Next ();
        REQUIRE (result.has_value ());
        patches.emplace_back (result.value ());
    }

    REQUIRE_EQUAL (patches.size (), 3u);
    CHECK_EQUAL (patches[0u].GetTypeMapping (), NonTrivialStruct::Reflect ().mapping);
    NonTrivialStruct nonTrivialStructLoaded;
    CHECK_NOT_EQUAL (nonTrivialStructLoaded, nonTrivialStructChanged);
    patches[0u].Apply (&nonTrivialStructLoaded);
    CHECK_EQUAL (nonTrivialStructLoaded, nonTrivialStructChanged);

    CHECK_EQUAL (patches[1u].GetTypeMapping (), TrivialStruct::Reflect ().mapping);
    TrivialStruct trivialStructLoaded;
    CHECK_NOT_EQUAL (trivialStructLoaded, trivialStructChanged);
    patches[1u].Apply (&trivialStructLoaded);
    CHECK_EQUAL (trivialStructLoaded, trivialStructChanged);

    CHECK_EQUAL (patches[2u].GetTypeMapping (), UnionStruct::Reflect ().mapping);
    UnionStruct unionStructLoaded;
    CHECK_NOT_EQUAL (unionStructLoaded, unionStructChanged);
    patches[2u].Apply (&unionStructLoaded);
    CHECK_EQUAL (unionStructLoaded, unionStructChanged);
}

END_SUITE

#undef PATCH_TESTS
