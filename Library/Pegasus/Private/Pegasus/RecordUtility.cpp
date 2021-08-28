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
    return DoWithCorrectComparator (_field,
                                    [_firstRecordValue, _secondRecordValue] (const auto &_comparator)
                                    {
                                        return _comparator.Compare (_firstRecordValue, _secondRecordValue) == 0;
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
    else if (first > second)
    {
        return 1;
    }
    else
    {
        return 0;
    }
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
} // namespace Emergence::Pegasus