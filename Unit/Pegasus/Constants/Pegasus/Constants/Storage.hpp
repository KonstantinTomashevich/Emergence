#pragma once

#include <cstdint>

#include <Pegasus/Constants/StorageProfile.hpp>

namespace Emergence::Pegasus::Constants::Storage
{
/// \brief Maximum count of indices with the same type (HashIndex, OrderedIndex or VolumetricIndex) within one storage.
constexpr std::size_t MAX_INDICES_OF_SAME_TYPE = Profile::Storage::MAX_INDICES_OF_SAME_TYPE;

/// \brief Maximum total count of fields, used by any index.
constexpr std::size_t MAX_INDEXED_FIELDS = Profile::Storage::MAX_INDEXED_FIELDS;

/// \brief Type of mask, that describes which fields index uses.
using IndexedFieldMask = Profile::Storage::IndexedFieldMask;

static_assert (sizeof (IndexedFieldMask) * 8u >= MAX_INDEXED_FIELDS);
} // namespace Emergence::Pegasus::Constants::Storage
