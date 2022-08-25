#include <sstream>

#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Test/Tests.hpp>
#include <Serialization/Test/Types.hpp>

#include <StandardLayout/PatchBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Serialization::Binary::Test
{
template <typename Type>
void ObjectSerializationDeserializationTest (const Type &_value)
{
    std::stringstream buffer;
    SerializeObject (buffer, &_value, Type::Reflect ().mapping);

    Type deserialized;
    CHECK (DeserializeObject (buffer, &deserialized, Type::Reflect ().mapping));
    CHECK_EQUAL (_value, deserialized);
}

template <typename Type>
void PatchSerializationDeserializationTest (const Type &_initial, const Type &_changed)
{
    std::stringstream buffer;
    SerializePatch (buffer,
                    StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &_initial));

    StandardLayout::PatchBuilder builder;
    CHECK (DeserializePatch (buffer, builder, Type::Reflect ().mapping));

    Type target = _initial;
    CHECK_NOT_EQUAL (target, _changed);
    builder.End ().Apply (&target);
    CHECK_EQUAL (target, _changed);
}

template <typename Type>
void FastPortablePatchSerializationDeserializationTest (const Type &_initial, const Type &_changed)
{
    std::stringstream buffer;
    SerializeFastPortablePatch (
        buffer, StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &_initial));

    FieldNameLookupCache cache {Type::Reflect ().mapping};
    StandardLayout::PatchBuilder builder;
    CHECK (DeserializeFastPortablePatch (buffer, builder, cache));

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

    serializer.Begin (buffer);
    for (const StandardLayout::Patch &patch : _patchesToSerialize)
    {
        serializer.Next (patch);
    }

    serializer.End ();

    PatchBundleDeserializer deserializer;
    deserializer.RegisterType (TrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (NonTrivialStruct::Reflect ().mapping);
    deserializer.RegisterType (UnionStruct::Reflect ().mapping);

    deserializer.Begin (buffer);
    while (deserializer.HasNext ())
    {
        Emergence::Container::Optional<Emergence::StandardLayout::Patch> result = deserializer.Next ();
        REQUIRE (result.has_value ());
        _deserializationOutput.emplace_back (result.value ());
    }

    deserializer.End ();
}
} // namespace Emergence::Serialization::Binary::Test

using namespace Emergence::Serialization::Test;
using namespace Emergence::Serialization::Binary::Test;

BEGIN_SUITE (BinaryObjectSerialization)

OBJECT_SERIALIZATION_TESTS (ObjectSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (BinaryPatchSerialization)

PATCH_SERIALIZATION_TESTS (PatchSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (BinaryFastPortablePatchSerialization)

PATCH_SERIALIZATION_TESTS (FastPortablePatchSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (BinaryPatchBundleSerialization)

PATCH_BUNDLE_SERIALIZATION_TESTS (PatchBundleSerializeAndDeserialize)

END_SUITE
