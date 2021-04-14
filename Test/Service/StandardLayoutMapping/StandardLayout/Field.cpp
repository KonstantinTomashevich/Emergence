#define BOOST_TEST_MODULE Field

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cstdint>

#include <StandardLayout/Field.hpp>
#include <ostream>

/// Helper suffix for integer to pointer type conversion.
uint8_t *operator "" _ptr (uintptr_t value)
{
    return reinterpret_cast <uint8_t *> (value);
}

// TODO: New Field class can not be created separately, therefore it can be tested only as part of whole service.

namespace Tests::FieldMeta::GetValue
{
//struct Dataset
//{
//    Emergence::StandardLayout::FieldMeta fieldMeta;
//    void *imaginaryObject;
//    void *expectedFieldAddress;
//
//    friend std::ostream &operator << (std::ostream &_output, const Dataset &_dataset) noexcept
//    {
//        _output << std::endl <<
//                "    fieldMeta: " << std::endl <<
//                "        offset: " << _dataset.fieldMeta.offset << std::endl;
//
//        if (_dataset.fieldMeta.archetype == Emergence::StandardLayout::FieldArchetype::BIT)
//        {
//            _output << "        bitOffset: " << _dataset.fieldMeta.bitOffset << std::endl;
//        }
//        else
//        {
//            _output << "        size: " << _dataset.fieldMeta.size << std::endl;
//        }
//
//        // TODO: Better archetype printing?
//        _output << "        archetype: " << static_cast <uint32_t> (_dataset.fieldMeta.archetype) << std::endl <<
//                "    imaginaryObject: " << reinterpret_cast <uintptr_t> (_dataset.imaginaryObject) << std::endl <<
//                "    expectedFieldAddress: " << reinterpret_cast <uintptr_t> (_dataset.expectedFieldAddress);
//        return _output;
//    }
//};
}

BOOST_AUTO_TEST_SUITE (FieldMeta)

//BOOST_DATA_TEST_CASE (
//    GetValue, boost::unit_test::data::monomorphic::collection (
//    std::vector <Tests::FieldMeta::GetValue::Dataset> {
//        {
//            {
//                .offset = 0u,
//                .bitOffset = 5u,
//                .archetype = Emergence::StandardLayout::FieldArchetype::BIT
//            },
//            0x10_ptr,
//            0x10_ptr
//        },
//        {
//            {
//                .offset = sizeof (uintptr_t) * 2u,
//                .size = sizeof (int),
//                .archetype = Emergence::StandardLayout::FieldArchetype::INT
//            },
//            0x10_ptr,
//            0x10_ptr + sizeof (uintptr_t) * 2u
//        },
//        {
//            {
//                .offset = sizeof (uintptr_t),
//                .size = sizeof (unsigned int),
//                .archetype = Emergence::StandardLayout::FieldArchetype::UINT
//            },
//            0x10_ptr,
//            0x10_ptr + sizeof (uintptr_t)
//        },
//        {
//            {
//                .offset = sizeof (uintptr_t) * 10u,
//                .size = sizeof (float),
//                .archetype = Emergence::StandardLayout::FieldArchetype::FLOAT
//            },
//            0x10_ptr,
//            0x10_ptr + sizeof (uintptr_t) * 10u
//        },
//        {
//            {
//                .offset = sizeof (uintptr_t) * 11u,
//                .size = 56u,
//                .archetype = Emergence::StandardLayout::FieldArchetype::STRING
//            },
//            0x10_ptr,
//            0x10_ptr + sizeof (uintptr_t) * 11u
//        },
//        {
//            {
//                .offset = sizeof (uintptr_t) * 6u,
//                .size = 128u,
//                .archetype = Emergence::StandardLayout::FieldArchetype::BLOCK
//            },
//            0x10_ptr,
//            0x10_ptr + sizeof (uintptr_t) * 6u
//        },
//    }))
//{
//    BOOST_CHECK (sample.fieldMeta.GetValue (sample.imaginaryObject) == sample.expectedFieldAddress);
//    BOOST_CHECK (sample.fieldMeta.GetValue (const_cast <const void *> (sample.imaginaryObject)) ==
//                 sample.expectedFieldAddress);
//}

BOOST_AUTO_TEST_SUITE_END ()
