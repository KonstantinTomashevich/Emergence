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

BEGIN_SUITE (YamlPatchBundleDeserialization)

TEST_CASE (DifferentTypes)
{
    static const char *yaml =
        "- type: NonTrivialStruct\n"
        "  content:\n"
        "    alive: true\n"
        "    uniqueString: \"For honor!\"\n"
        "- type: TrivialStruct\n"
        "  content:\n"
        "    int8: -32\n"
        "    uint32: 1537\n"
        "- type: UnionStruct\n"
        "  content:\n"
        "    type: 1\n"
        "    m: 4\n"
        "    n: 17\n";

    std::stringstream buffer;
    buffer << yaml;

    BundleDeserializationContext context;
    context.RegisterType (TrivialStruct::Reflect ().mapping);
    context.RegisterType (NonTrivialStruct::Reflect ().mapping);
    context.RegisterType (UnionStruct::Reflect ().mapping);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> patches;
    DeserializePatchBundle (buffer, patches, context);

    REQUIRE_EQUAL (patches.size (), 3u);

    CHECK_EQUAL (patches[0u].GetTypeMapping (), NonTrivialStruct::Reflect ().mapping);
    {
        auto iterator = patches[0u].Begin ();
        REQUIRE_NOT_EQUAL (iterator, patches[0u].End ());
        CHECK_EQUAL ((*iterator).field, NonTrivialStruct::Reflect ().alive);
        CHECK_EQUAL (*static_cast<const bool *> ((*iterator).newValue), true);

        ++iterator;
        REQUIRE_NOT_EQUAL (iterator, patches[0u].End ());
        CHECK_EQUAL ((*iterator).field, NonTrivialStruct::Reflect ().uniqueString);
        CHECK_EQUAL (*static_cast<const Emergence::Memory::UniqueString *> ((*iterator).newValue),
                     Emergence::Memory::UniqueString {"For honor!"});

        ++iterator;
        CHECK_EQUAL (iterator, patches[0u].End ());
    }

    CHECK_EQUAL (patches[1u].GetTypeMapping (), TrivialStruct::Reflect ().mapping);
    {
        auto iterator = patches[1u].Begin ();
        REQUIRE_NOT_EQUAL (iterator, patches[1u].End ());
        CHECK_EQUAL ((*iterator).field, TrivialStruct::Reflect ().int8);
        CHECK_EQUAL (*static_cast<const int8_t *> ((*iterator).newValue), -32);

        ++iterator;
        REQUIRE_NOT_EQUAL (iterator, patches[1u].End ());
        CHECK_EQUAL ((*iterator).field, TrivialStruct::Reflect ().uint32);
        CHECK_EQUAL (*static_cast<const uint32_t *> ((*iterator).newValue), 1537u);

        ++iterator;
        CHECK_EQUAL (iterator, patches[1u].End ());
    }

    CHECK_EQUAL (patches[2u].GetTypeMapping (), UnionStruct::Reflect ().mapping);
    {
        auto iterator = patches[2u].Begin ();
        REQUIRE_NOT_EQUAL (iterator, patches[2u].End ());
        CHECK_EQUAL ((*iterator).field, UnionStruct::Reflect ().type);
        CHECK_EQUAL (*static_cast<const uint64_t  *> ((*iterator).newValue), 1u);

        ++iterator;
        REQUIRE_NOT_EQUAL (iterator, patches[2u].End ());
        CHECK_EQUAL ((*iterator).field, UnionStruct::Reflect ().m);
        CHECK_EQUAL (*static_cast<const uint64_t  *> ((*iterator).newValue), 4u);

        ++iterator;
        REQUIRE_NOT_EQUAL (iterator, patches[2u].End ());
        CHECK_EQUAL ((*iterator).field, UnionStruct::Reflect ().n);
        CHECK_EQUAL (*static_cast<const uint64_t  *> ((*iterator).newValue), 17u);

        ++iterator;
        CHECK_EQUAL (iterator, patches[2u].End ());
    }
}

END_SUITE

#undef PATCH_TESTS
