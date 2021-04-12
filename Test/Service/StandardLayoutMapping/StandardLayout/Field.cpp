#define BOOST_TEST_MODULE Field

#include <boost/test/unit_test.hpp>

#include <cstdint>

#include <StandardLayout/Field.hpp>

using namespace Emergence::StandardLayout;

/// Helper suffix for integer to pointer type conversion.
uint8_t *operator "" _ptr (uintptr_t value)
{
    return reinterpret_cast <uint8_t *> (value);
}

namespace Tests::Field::GetValue
{
static void Execute (FieldMeta _fieldMeta, void *_imaginaryObject, const void *_expectedFieldAddress)
{
    BOOST_CHECK (_fieldMeta.GetValue (_imaginaryObject) == _expectedFieldAddress);
    BOOST_CHECK (_fieldMeta.GetValue (const_cast <const void *> (_imaginaryObject)) == _expectedFieldAddress);
}
}

BOOST_AUTO_TEST_SUITE (Field)

BOOST_AUTO_TEST_CASE (BitAtStart)
{
    Tests::Field::GetValue::Execute (
        {.offset = 0u, .bitOffset = 5u, .archetype = FieldArchetype::BIT},
        0x10_ptr,
        0x10_ptr);
}

BOOST_AUTO_TEST_CASE (ThirdInt)
{
    Tests::Field::GetValue::Execute (
        {.offset = sizeof (uintptr_t) * 2u, .size = sizeof (int), .archetype = FieldArchetype::INT},
        0x10_ptr,
        0x10_ptr + sizeof (uintptr_t) * 2u);
}

BOOST_AUTO_TEST_CASE (SecondUInt)
{
    Tests::Field::GetValue::Execute (
        {.offset = sizeof (uintptr_t), .size = sizeof (unsigned int), .archetype = FieldArchetype::UINT},
        0x10_ptr,
        0x10_ptr + sizeof (uintptr_t));
}

BOOST_AUTO_TEST_CASE (EleventhFloat)
{
    Tests::Field::GetValue::Execute (
        {.offset = sizeof (uintptr_t) * 10u, .size = sizeof (float), .archetype = FieldArchetype::FLOAT},
        0x10_ptr,
        0x10_ptr + sizeof (uintptr_t) * 10u);
}

BOOST_AUTO_TEST_CASE (TwelfthString)
{
    Tests::Field::GetValue::Execute (
        {.offset = sizeof (uintptr_t) * 11u, .size = 56u, .archetype = FieldArchetype::STRING},
        0x10_ptr,
        0x10_ptr + sizeof (uintptr_t) * 11u);
}

BOOST_AUTO_TEST_CASE (SeventhBlock)
{
    Tests::Field::GetValue::Execute (
        {.offset = sizeof (uintptr_t) * 6u, .size = 128u, .archetype = FieldArchetype::BLOCK},
        0x10_ptr,
        0x10_ptr + sizeof (uintptr_t) * 6u);
}

BOOST_AUTO_TEST_SUITE_END ()
