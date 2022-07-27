#include <sstream>

#include <Container/InplaceVector.hpp>

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

#define PATCH_TEST_ROUTINE(PatchType)                                                                                  \
    template <typename Type>                                                                                           \
    void PatchType##SerializationDeserializationTest (const Type &_initial, const Type &_changed)                      \
    {                                                                                                                  \
        std::stringstream buffer;                                                                                      \
        Serialize##PatchType (                                                                                         \
            buffer, StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &_initial));    \
                                                                                                                       \
        StandardLayout::PatchBuilder builder;                                                                          \
        CHECK (Deserialize##PatchType (buffer, builder, Type::Reflect ().mapping));                                    \
                                                                                                                       \
        Type target = _initial;                                                                                        \
        CHECK_NOT_EQUAL (target, _changed);                                                                            \
        builder.End ().Apply (&target);                                                                                \
        CHECK_EQUAL (target, _changed);                                                                                \
    }

PATCH_TEST_ROUTINE (Patch)
PATCH_TEST_ROUTINE (FastPortablePatch)

#undef PATCH_TEST_ROUTINE
} // namespace Emergence::Serialization::Binary::Test

using namespace Emergence::Serialization::Test;
using namespace Emergence::Serialization::Binary::Test;

BEGIN_SUITE (BinaryObjectSerialization)

OBJECT_SERIALIZATION_TESTS (ObjectSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (BinaryPatchSerialization)

PATCH_SERIALIZATION_TESTS(PatchSerializationDeserializationTest)

END_SUITE

BEGIN_SUITE (BinaryFastPortablePatchSerialization)

PATCH_SERIALIZATION_TESTS(FastPortablePatchSerializationDeserializationTest)

END_SUITE

#undef PATCH_TESTS
