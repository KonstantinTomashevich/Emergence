#pragma once

#include <cstdint>
#include <string_view>

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Memory
{
class UniqueString final
{
public:
    UniqueString () noexcept;

    explicit UniqueString (const char *_string) noexcept;

    explicit UniqueString (const std::string_view &_string) noexcept;

    /// All implementations must be trivially copyable.
    UniqueString (const UniqueString &_other) noexcept = default;

    /// All implementations must be trivially movable.
    UniqueString (UniqueString &&_other) noexcept = default;

    /// All implementations must be trivially destructible.
    ~UniqueString () noexcept = default;

    [[nodiscard]] const char *operator* () const noexcept;

    [[nodiscard]] uintptr_t Hash () const noexcept;

    [[nodiscard]] bool operator== (const UniqueString &_other) const noexcept;

    [[nodiscard]] bool operator!= (const UniqueString &_other) const noexcept;

    /// All implementations must be trivially copy assignable.
    UniqueString &operator= (const UniqueString &_other) noexcept = default;

    /// All implementations must be trivially move assignable.
    UniqueString &operator= (UniqueString &&_other) noexcept = default;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));
};

namespace Literals
{
inline UniqueString operator"" _us (const char *_string, [[maybe_unused]] size_t _size)
{
    return UniqueString {_string};
}
} // namespace Literals
} // namespace Emergence::Memory

namespace std
{
template <>
struct hash<Emergence::Memory::UniqueString>
{
    std::size_t operator() (const Emergence::Memory::UniqueString &_string) const noexcept
    {
        return reinterpret_cast<std::size_t> (_string.Hash ());
    }
};
} // namespace std
