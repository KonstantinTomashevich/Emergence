#pragma once

#include <string>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
using String = std::basic_string<char, std::char_traits<char>, Memory::HeapSTD<char>>;
} // namespace Emergence::Container

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (char)
