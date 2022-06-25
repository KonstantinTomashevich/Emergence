#pragma once

#include <cstdint>
#include <numeric>

namespace Emergence::Celerity
{
/// \brief Suggested value type for storing unique ids, for example objects ids.
using UniqueId = std::uint64_t;

/// \brief Suggest value for indication of invalid or not-specified unique ids.
constexpr UniqueId INVALID_UNIQUE_ID = std::numeric_limits<UniqueId>::max ();
} // namespace Emergence::Celerity
