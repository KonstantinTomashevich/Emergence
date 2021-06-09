#pragma once

#include <StandardLayout/Field.hpp>

namespace Emergence::Pegasus
{
bool AreFieldValuesEqual (const void *_firstRecord, const void *_secondRecord,
                          const StandardLayout::Field &_field) noexcept;
} // namespace Emergence::Pegasus
