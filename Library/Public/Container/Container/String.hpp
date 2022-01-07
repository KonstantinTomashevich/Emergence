#pragma once

#include <string>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
using String = std::basic_string<char, std::char_traits<char>, Memory::HeapSTD<char>>;

/// \return Decimal representation of given number.
String ToString (int _value);

/// \return Decimal representation of given number.
String ToString (long _value);

/// \return Decimal representation of given number.
String ToString (long long _value);

/// \return Decimal representation of given number.
String ToString (unsigned _value);

/// \return Decimal representation of given number.
String ToString (unsigned long _value);

/// \return Decimal representation of given number.
String ToString (unsigned long long _value);

/// \return Decimal representation of given number.
String ToString (float _value);

/// \return Decimal representation of given number.
String ToString (double _value);

/// \return Decimal representation of given number.
String ToString (long double _value);

namespace Literals
{
inline String operator"" _s (const char *_string, size_t _size)
{
    return String {_string, _size};
}
} // namespace Literals
} // namespace Emergence::Container

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (char)
