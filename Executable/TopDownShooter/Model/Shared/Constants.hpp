#pragma once

#include <cstdint>
#include <numeric>

// TODO: IDs are OK, but soon we might need to introduce weak pointers.
constexpr std::uintptr_t INVALID_OBJECT_ID = std::numeric_limits<std::uintptr_t>::max ();
