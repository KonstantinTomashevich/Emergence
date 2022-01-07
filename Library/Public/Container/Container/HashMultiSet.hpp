#pragma once

#include <unordered_set>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename T, typename Hasher = std::hash<T>, typename Comparator = std::equal_to<T>>
using HashMultiSet = std::unordered_multiset<T, Hasher, Comparator, Memory::HeapSTD<T>>;
} // namespace Emergence::Container
