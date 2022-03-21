#pragma once

#include <cstdint>
#include <numeric>

namespace Emergence::Celerity
{
// TODO: Should it really be part of Celerity?
using ObjectId = std::uint64_t;

constexpr ObjectId INVALID_OBJECT_ID = std::numeric_limits<ObjectId>::max ();
} // namespace Emergence::Celerity
