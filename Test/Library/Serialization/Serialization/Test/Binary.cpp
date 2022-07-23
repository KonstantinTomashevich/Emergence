#include <sstream>

#include <Container/InplaceVector.hpp>

#include <Serialization/Binary.hpp>

#include <StandardLayout/MappingRegistration.hpp>
#include <StandardLayout/PatchBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Serialization::Binary::Test
{
struct DirectlySerializableStruct final
{
    int8_t int8;
    int16_t int16;
    int32_t int32;
    int64_t int64;

    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;
    uint64_t uint64;

    float floating;
    double doubleFloating;

    std::array<uint8_t, 8u> data;

    bool operator== (const DirectlySerializableStruct &_other) const noexcept = default;

    bool operator!= (const DirectlySerializableStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId int8;
        StandardLayout::FieldId int16;
        StandardLayout::FieldId int32;
        StandardLayout::FieldId int64;

        StandardLayout::FieldId uint8;
        StandardLayout::FieldId uint16;
        StandardLayout::FieldId uint32;
        StandardLayout::FieldId uint64;

        StandardLayout::FieldId floating;
        StandardLayout::FieldId doubleFloating;

        StandardLayout::FieldId data;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const DirectlySerializableStruct::Reflection &DirectlySerializableStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DirectlySerializableStruct);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (uint8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTER_REGULAR (doubleFloating);

        EMERGENCE_MAPPING_REGISTER_BLOCK (data);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct SpecialCasesStruct final
{
    static constexpr uint8_t ALIVE_OFFSET = 0u;
    static constexpr uint8_t POISONED_OFFSET = 1u;
    static constexpr uint8_t STUNNED_OFFSET = 2u;

    uint8_t flags = 0u;
    std::array<char, 32u> string {'\0'};
    Memory::UniqueString uniqueString {"Hello, world!"};

    bool operator== (const SpecialCasesStruct &_other) const noexcept;

    bool operator!= (const SpecialCasesStruct &_other) const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId string;
        StandardLayout::FieldId uniqueString;
        StandardLayout::FieldId alive;
        StandardLayout::FieldId poisoned;
        StandardLayout::FieldId stunned;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

bool SpecialCasesStruct::operator== (const SpecialCasesStruct &_other) const noexcept
{
    constexpr uint8_t USED_BITS = (1u << ALIVE_OFFSET) | (1u << POISONED_OFFSET) | (1u << STUNNED_OFFSET);
    return (flags & USED_BITS) == (_other.flags & USED_BITS) && strcmp (string.data (), _other.string.data ()) == 0 &&
           uniqueString == _other.uniqueString;
}

bool SpecialCasesStruct::operator!= (const SpecialCasesStruct &_other) const noexcept
{
    return !(*this == _other);
}

const SpecialCasesStruct::Reflection &SpecialCasesStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SpecialCasesStruct);
        EMERGENCE_MAPPING_REGISTER_BIT (alive, flags, ALIVE_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (poisoned, flags, POISONED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (stunned, flags, STUNNED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_STRING (string);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniqueString);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct UnionStruct final
{
    uint64_t type = 0u;
    union
    {
        struct
        {
            float x;
            float y;
        };

        struct
        {
            uint64_t m;
            uint64_t n;
        };
    };

    bool operator== (const UnionStruct &_other) const noexcept;

    bool operator!= (const UnionStruct &_other) const noexcept;

    struct Reflection
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId m;
        StandardLayout::FieldId n;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

bool UnionStruct::operator== (const UnionStruct &_other) const noexcept
{
    if (type != _other.type)
    {
        return false;
    }

    if (type == 0u)
    {
        return x == _other.x && y == _other.y;
    }

    REQUIRE_EQUAL (type, 1u);
    return m == _other.m && n == _other.n;
}

bool UnionStruct::operator!= (const UnionStruct &_other) const noexcept
{
    return !(*this == _other);
}

const UnionStruct::Reflection &UnionStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UnionStruct);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m);
        EMERGENCE_MAPPING_REGISTER_REGULAR (n);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

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
} // namespace Emergence::Serialization::Binary::Test

using namespace Emergence::Serialization::Binary::Test;

BEGIN_SUITE (Binary)

TEST_CASE (Direct)
{
    ObjectSerializationDeserializationTest (DirectlySerializableStruct {
        -19, 163, -2937, 1123, 21u, 784u, 17274u, 18274u, 1.20338f, 5647.385639, {11u, 79u, 54u, 121u}});
}

TEST_CASE (SpecialCases)
{
    ObjectSerializationDeserializationTest (SpecialCasesStruct {
        (1u << SpecialCasesStruct::ALIVE_OFFSET) | (1u << SpecialCasesStruct::STUNNED_OFFSET),
        {"Let's test this code!\0"},
        Emergence::Memory::UniqueString {"For glory and gold!"},
    });
}

TEST_CASE (Union)
{
    UnionStruct first;
    first.type = 0u;
    first.x = 1.647f;
    first.y = 173.129337f;
    ObjectSerializationDeserializationTest (first);

    UnionStruct second;
    second.type = 1u;
    second.m = 172947923u;
    second.n = 123838471u;
    ObjectSerializationDeserializationTest (second);
}

TEST_CASE (InplaceVector)
{
    Emergence::Container::InplaceVector<UnionStruct, 4u> vector;
    UnionStruct &first = vector.EmplaceBack ();
    first.type = 0u;
    first.x = 1.647f;
    first.y = 173.129337f;

    UnionStruct &second = vector.EmplaceBack ();
    second.type = 1u;
    second.m = 172947923u;
    second.n = 123838471u;
    ObjectSerializationDeserializationTest (vector);
}

TEST_CASE (DirectPatch)
{
    PatchSerializationDeserializationTest<DirectlySerializableStruct> (
        // Because array data is registered as block which is
        // not supported by patches, we are filling it with zeros.
        {-19, 163, -2937, 1123, 21u, 784u, 17274u, 18274u, 1.20338f, 5647.385639, {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},
        {-20, 164, -2936, 4512, 17u, 783u, 26172u, 18271u, 1.20339f, 5648.385639, {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}});
}

TEST_CASE (SpecialCasesPatch)
{
    PatchSerializationDeserializationTest<SpecialCasesStruct> (
        {
            (1u << SpecialCasesStruct::ALIVE_OFFSET) | (1u << SpecialCasesStruct::STUNNED_OFFSET),
            // Strings are zeroed because they are not supported by patches.
            {"\0"},
            Emergence::Memory::UniqueString {"For glory and gold!"},
        },
        {
            (1u << SpecialCasesStruct::ALIVE_OFFSET),
            {"\0"},
            Emergence::Memory::UniqueString {"Hello, world!"},
        });
}

TEST_CASE (UnionPatch)
{
    UnionStruct first;
    first.type = 0u;
    first.x = 1.647f;
    first.y = 173.129337f;

    UnionStruct second;
    second.type = 1u;
    second.m = 172947923u;
    second.n = 123838471u;
    PatchSerializationDeserializationTest (first, second);
}

END_SUITE
