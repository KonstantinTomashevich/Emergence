#pragma once

#define BOOST_NO_EXCEPTIONS
#include <boost/unordered_map.hpp>

#include <Container/BoostExceptionMock.hpp>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename Key, typename Value, typename Hasher = std::hash<Key>, typename Comparator = std::equal_to<Key>>
using HashMap = boost::unordered_map<Key, Value, Hasher, Comparator, Memory::HeapSTD<std::pair<const Key, Value>>>;
} // namespace Emergence::Container
