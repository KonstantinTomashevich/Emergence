#pragma once

#include <cstdint>

#include <Pegasus/Constants/Implementation/Storage.hpp>

namespace Emergence::Pegasus::Constants::Storage
{
/// \brief Maximum count of indices with the same type (HashIndex, OrderedIndex or VolumetricIndex) within one storage.
constexpr std::size_t MAX_INDICES_OF_SAME_TYPE = Implementation::Storage::MAX_INDICES_OF_SAME_TYPE;

/// \brief Maximum total count of fields, used by any index.
constexpr std::size_t MAX_INDEXED_FIELDS = Implementation::Storage::MAX_INDEXED_FIELDS;

/// \brief Type of mask, that describes which fields index uses.
using IndexedFieldMask = Implementation::Storage::IndexedFieldMask;

static_assert (sizeof (IndexedFieldMask) * 8u >= MAX_INDEXED_FIELDS);
} // namespace Emergence::Pegasus::Constants::Storage
