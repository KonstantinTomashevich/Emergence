#pragma once

#include <cstdint>

#include <Pegasus/Constants/HashIndexProfile.hpp>

namespace Emergence::Pegasus::Constants::HashIndex
{
/// \brief Maximum count of fields for one HashIndex.
constexpr std::size_t MAX_INDEXED_FIELDS = Profile::HashIndex::MAX_INDEXED_FIELDS;
} // namespace Emergence::Pegasus::Constants::HashIndex
