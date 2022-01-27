#include <cassert>
#include <cstring>

#include <Pegasus/RecordUtility.hpp>

namespace Emergence::Pegasus
{
bool AreRecordValuesEqual (const void *_firstRecord,
                           const void *_secondRecord,
                           const StandardLayout::Field &_field) noexcept
{
    assert (_field.IsHandleValid ());
    return AreFieldValuesEqual (_field.GetValue (_firstRecord), _field.GetValue (_secondRecord), _field);
}

bool AreFieldValuesEqual (const void *_firstRecordValue,
                          const void *_secondRecordValue,
                          const StandardLayout::Field &_field) noexcept
{
    assert (_field.IsHandleValid ());
    return DoWithCorrectComparator (
        _field,
        [_firstRecordValue, _secondRecordValue] (const auto &_comparator)
        {
            // Here we are just checking equality instead of comparing.
            // Unique string equality check is much faster than generic comparison.
            if constexpr (std::is_same_v<std::decay_t<decltype (_comparator)>, UniqueStringValueComparator>)
            {
                return *static_cast<const Memory::UniqueString *> (_firstRecordValue) ==
                       *static_cast<const Memory::UniqueString *> (_secondRecordValue);
            }
            else
            {
                return _comparator.Compare (_firstRecordValue, _secondRecordValue) == 0;
            }
        });
}

int BitValueComparator::Compare (const void *_firstValue, const void *_secondValue) const noexcept
{
    uint8_t first = *static_cast<const uint8_t *> (_firstValue) & mask;
    uint8_t second = *static_cast<const uint8_t *> (_secondValue) & mask;

    if (first < second)
    {
        return -1;
    }

    if (first > second)
    {
        return 1;
    }

    return 0;
}

int BlockValueComparator::Compare (const void *_firstValue, const void *_secondValue) const noexcept
{
    return memcmp (_firstValue, _secondValue, size);
}

int StringValueComparator::Compare (const void *_firstValue, const void *_secondValue) const noexcept
{
    return strncmp (static_cast<const char *> (_firstValue), static_cast<const char *> (_secondValue),
                    maxSize / sizeof (char));
}

int UniqueStringValueComparator::Compare (const void *_firstValue, const void *_secondValue) noexcept
{
    const Memory::UniqueString &first = *static_cast<const Memory::UniqueString *> (_firstValue);
    const Memory::UniqueString &second = *static_cast<const Memory::UniqueString *> (_secondValue);
    return strcmp (*first, *second);
}
} // namespace Emergence::Pegasus
