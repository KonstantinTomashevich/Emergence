#pragma once

#include <vector>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
template <typename T>
using Vector = std::vector<T, Memory::HeapSTD<T>>;

/// \brief Erases vector element by exchanging it with last element and then popping last element.
/// \details This erasure method has better performance, because it avoids unnecessary copying,
///          but it also breaks vector order.
template <typename Vector, typename Iterator>
Iterator EraseExchangingWithLast (Vector &_vector, const Iterator &_iterator)
{
    auto last = --_vector.end ();
    if (_iterator != last)
    {
        *_iterator = std::move (*last);
    }

    _vector.pop_back ();
    return _iterator;
}
} // namespace Emergence::Container
