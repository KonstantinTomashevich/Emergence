#include <Serialization/Test/Types.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Serialization::Test
{
const TrivialStruct::Reflection &TrivialStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TrivialStruct);
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

bool NonTrivialStruct::operator== (const NonTrivialStruct &_other) const noexcept
{
    constexpr uint8_t USED_BITS = (1u << ALIVE_OFFSET) | (1u << POISONED_OFFSET) | (1u << STUNNED_OFFSET);
    return (flags & USED_BITS) == (_other.flags & USED_BITS) && strcmp (string.data (), _other.string.data ()) == 0 &&
           uniqueString == _other.uniqueString;
}

bool NonTrivialStruct::operator!= (const NonTrivialStruct &_other) const noexcept
{
    return !(*this == _other);
}

const NonTrivialStruct::Reflection &NonTrivialStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (NonTrivialStruct);
        EMERGENCE_MAPPING_REGISTER_BIT (alive, flags, ALIVE_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (poisoned, flags, POISONED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (stunned, flags, STUNNED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_STRING (string);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniqueString);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

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
} // namespace Emergence::Serialization::Test
