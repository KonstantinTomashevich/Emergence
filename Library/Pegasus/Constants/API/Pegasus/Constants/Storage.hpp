#pragma once

#include <cstdint>

#include <Pegasus/Constants/Implementation/Storage.hpp>

namespace Emergence::Pegasus::Constants::Storage
{
static constexpr std::size_t MAX_INDICES_OF_SAME_TYPE = Implementation::Storage::MAX_INDICES_OF_SAME_TYPE;

static constexpr std::size_t MAX_INDEXED_FIELDS = Implementation::Storage::MAX_INDEXED_FIELDS;

using IndexedFieldMask = Implementation::Storage::IndexedFieldMask;

static_assert (sizeof (IndexedFieldMask) * 8u >= MAX_INDEXED_FIELDS);
} // namespace Emergence::Pegasus::Constants::Storage
