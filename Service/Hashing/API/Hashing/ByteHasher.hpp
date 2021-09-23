#pragma once

#include <array>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Hashing
{
/// \brief Stateful hasher for byte sequences.
class ByteHasher final
{
public:
    ByteHasher () noexcept;

    ByteHasher (const ByteHasher &_other) noexcept;

    ByteHasher (ByteHasher &&_other) noexcept;

    ~ByteHasher () noexcept;

    /// \brief Modifies current state by appending given byte.
    void Append (uint8_t _byte) noexcept;

    /// \brief Modifies current state by appending given byte sequence.
    void Append (const uint8_t *_bytes, std::size_t _count) noexcept;

    /// \brief Resets current state to initial values.
    void Clear () noexcept;

    /// \return 64-bit hash value for current state.
    [[nodiscard]] uint64_t GetCurrentValue () const noexcept;

    ByteHasher &operator= (const ByteHasher &_other) noexcept;

    ByteHasher &operator= (ByteHasher &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 10u);
};
} // namespace Emergence::Hashing
