#pragma once

#define BOOST_NO_EXCEPTIONS
#include <boost/unordered_set.hpp>

#include <Container/BoostExceptionMock.hpp>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename T, typename Hasher = std::hash<T>, typename Comparator = std::equal_to<T>>
using HashMultiSet = boost::unordered_multiset<T, Hasher, Comparator, Memory::HeapSTD<T>>;
} // namespace Emergence::Container
