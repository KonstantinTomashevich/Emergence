#pragma once

#include <cstdint>

namespace Emergence::Pegasus::Constants::Implementation::Storage
{
constexpr std::size_t MAX_INDICES_OF_SAME_TYPE = 8u;

constexpr std::size_t MAX_INDEXED_FIELDS = 32u;

using IndexedFieldMask = uint_fast32_t;
} // namespace Emergence::Pegasus::Constants::Implementation::Storage
