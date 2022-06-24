#pragma once

#include <cstdint>
#include <numeric>

namespace Emergence::Celerity
{
using UniqueId = std::uint64_t;

constexpr UniqueId INVALID_UNIQUE_ID = std::numeric_limits<UniqueId>::max ();
} // namespace Emergence::Celerity
