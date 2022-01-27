#pragma once

#include <unordered_map>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename Key, typename Value, typename Hasher = std::hash<Key>, typename Comparator = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, Value, Hasher, Comparator, Memory::HeapSTD<std::pair<const Key, Value>>>;
} // namespace Emergence::Container
