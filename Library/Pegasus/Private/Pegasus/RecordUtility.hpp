#pragma once

#include <StandardLayout/Field.hpp>

namespace Emergence::Pegasus
{
bool AreRecordValuesEqual (const void *_firstRecord, const void *_secondRecord,
                           const StandardLayout::Field &_field) noexcept;

bool AreFieldValuesEqual (const void *_firstRecordValue, const void *_secondRecordValue,
                          const StandardLayout::Field &_field) noexcept;
} // namespace Emergence::Pegasus
