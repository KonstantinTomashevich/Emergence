#pragma once

#include <vector>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename T>
using Vector = std::vector<T, Memory::HeapSTD<T>>;
} // namespace Emergence::Container
