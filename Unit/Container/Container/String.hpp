#pragma once

#include <ContainerApi.hpp>

#include <string>

#include <Memory/Heap.hpp>

// NOLINTNEXTLINE(modernize-concat-nested-namespaces): False positive?
namespace Emergence::Container
{
using String = std::basic_string<char, std::char_traits<char>, Memory::HeapSTD<char>>;

using Utf8String = std::basic_string<char, std::char_traits<char>, Memory::HeapSTD<char>>;

namespace Literals
{
inline String operator"" _s (const char *_string, [[maybe_unused]] std::size_t _size)
{
    return String {_string};
}
} // namespace Literals
} // namespace Emergence::Container

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (ContainerApi, char)

// Some compilers do not support hashing for strings with custom allocator,
// therefore we have to add redirect to string view.
namespace std
{
template <>
struct hash<Emergence::Container::String>
{
    std::size_t operator() (const Emergence::Container::String &_string) const noexcept
    {
        return std::hash<std::string_view> {}(std::string_view {_string});
    }
};
} // namespace std
