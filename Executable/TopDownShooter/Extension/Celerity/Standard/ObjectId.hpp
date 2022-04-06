#pragma once

#include <cstdint>
#include <numeric>

namespace Emergence::Celerity
{
using ObjectId = std::uint64_t;

constexpr ObjectId INVALID_OBJECT_ID = std::numeric_limits<ObjectId>::max ();
} // namespace Emergence::Celerity
