#pragma once

#include <string>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
using String = std::basic_string<char, std::char_traits<char>, Memory::HeapSTD<char>>;

namespace Literals
{
inline String operator"" _s (const char *_string, [[maybe_unused]] size_t _size)
{
    return String {_string};
}
} // namespace Literals
} // namespace Emergence::Container

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (char)
