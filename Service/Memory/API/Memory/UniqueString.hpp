#pragma once

#include <cstdint>
#include <string_view>

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Memory
{
/// \brief Provides fast comparison, hashing and trivial destruction for immutable strings.
class UniqueString final
{
public:
    /// \brief Constructs immutable empty string.
    UniqueString () noexcept;

    /// \brief Constructs immutable string from given null-terminated string.
    /// \details If there is no UniqueString instance with equal content, then content of given string will be copied.
    explicit UniqueString (const char *_string) noexcept;

    /// \brief Constructs immutable string from given string.
    /// \details If there is no UniqueString instance with equal content, then content of given string will be copied.
    explicit UniqueString (const std::string_view &_string) noexcept;

    /// All implementations must be trivially copyable.
    UniqueString (const UniqueString &_other) noexcept = default;

    /// All implementations must be trivially movable.
    UniqueString (UniqueString &&_other) noexcept = default;

    /// All implementations must be trivially destructible.
    ~UniqueString () noexcept = default;

    /// \return Pointer to stored null-terminated string.
    [[nodiscard]] const char *operator* () const noexcept;

    /// \return Hash of stored string. Guaranteed to be unique,
    ///         but can not be stored in files or shared through processes.
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
        return static_cast<std::size_t> (_string.Hash ());
    }
};
} // namespace std
