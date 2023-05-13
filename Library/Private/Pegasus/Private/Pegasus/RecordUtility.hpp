#pragma once

#include <compare>
#include <type_traits>

#include <Assert/Assert.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::Pegasus
{
/// \brief Wrapper for ::DoWithCorrectComparator, that applies comparator to fields of two given records.
///
/// \details If you have more than two records to compare, consider using ::DoWithCorrectComparator.
bool AreRecordValuesEqual (const void *_firstRecord,
                           const void *_secondRecord,
                           const StandardLayout::Field &_field) noexcept;

/// \brief Wrapper for ::DoWithCorrectComparator, that applies comparator to two given values.
///
/// \details If you have more than two values to compare, consider using ::DoWithCorrectComparator.
bool AreFieldValuesEqual (const void *_firstRecordValue,
                          const void *_secondRecordValue,
                          const StandardLayout::Field &_field) noexcept;

/// \brief Select appropriate value comparator for given field and passes it as argument to callback.
template <typename Callback>
auto DoWithCorrectComparator (const StandardLayout::Field &_field, const Callback &_callback) noexcept;

template <typename Type>
struct NumericValueComparator
{
    int Compare (const void *_firstValue, const void *_secondValue) const noexcept;
};

struct BitValueComparator final
{
    int Compare (const void *_firstValue, const void *_secondValue) const noexcept;

    const uint8_t mask;
};

struct BlockValueComparator final
{
    int Compare (const void *_firstValue, const void *_secondValue) const noexcept;

    const std::size_t size;
};

struct StringValueComparator final
{
    int Compare (const void *_firstValue, const void *_secondValue) const noexcept;

    const std::size_t maxSize;
};

struct UniqueStringValueComparator final
{
    static int Compare (const void *_firstValue, const void *_secondValue) noexcept;
};

template <typename Callback>
auto DoWithCorrectComparator (const StandardLayout::Field &_field, const Callback &_callback) noexcept
{
    EMERGENCE_ASSERT (_field.IsHandleValid ());
    switch (_field.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:
    {
        uint8_t mask = 1u << _field.GetBitOffset ();
        return _callback (BitValueComparator {mask});
    }

    case StandardLayout::FieldArchetype::INT:
    {
        switch (_field.GetSize ())
        {
        case sizeof (int8_t):
            return _callback (NumericValueComparator<int8_t> {});

        case sizeof (int16_t):
            return _callback (NumericValueComparator<int16_t> {});

        case sizeof (int32_t):
            return _callback (NumericValueComparator<int32_t> {});

        case sizeof (int64_t):
            return _callback (NumericValueComparator<int64_t> {});
        }

        break;
    }
    case StandardLayout::FieldArchetype::UINT:
    {
        switch (_field.GetSize ())
        {
        case sizeof (uint8_t):
            return _callback (NumericValueComparator<uint8_t> {});

        case sizeof (uint16_t):
            return _callback (NumericValueComparator<uint16_t> {});

        case sizeof (uint32_t):
            return _callback (NumericValueComparator<uint32_t> {});

        case sizeof (uint64_t):
            return _callback (NumericValueComparator<uint64_t> {});
        }

        break;
    }

    case StandardLayout::FieldArchetype::FLOAT:
    {
        switch (_field.GetSize ())
        {
        case sizeof (float):
            return _callback (NumericValueComparator<float> {});

        case sizeof (double):
            return _callback (NumericValueComparator<double> {});
        }

        break;
    }

    case StandardLayout::FieldArchetype::BLOCK:
    {
        return _callback (BlockValueComparator {_field.GetSize ()});
    }

    case StandardLayout::FieldArchetype::STRING:
    {
        return _callback (StringValueComparator {_field.GetSize ()});
    }

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
    {
        return _callback (UniqueStringValueComparator {});
    }

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
    {
        // _field should be leaf-field, not intermediate nested object.
        EMERGENCE_ASSERT (false);
    }

    case StandardLayout::FieldArchetype::VECTOR:
    case StandardLayout::FieldArchetype::PATCH:
    {
        // Vectors and patches aren't supported for indexing.
        EMERGENCE_ASSERT (false);
    }
    }

    EMERGENCE_ASSERT (false);
    return _callback (BlockValueComparator {_field.GetSize ()});
}

template <typename Type>
int NumericValueComparator<Type>::Compare (const void *_firstValue, const void *_secondValue) const noexcept
{
    if (*static_cast<const Type *> (_firstValue) < *static_cast<const Type *> (_secondValue))
    {
        return -1;
    }

    if (*static_cast<const Type *> (_firstValue) > *static_cast<const Type *> (_secondValue))
    {
        return 1;
    }

    return 0;
}
} // namespace Emergence::Pegasus
