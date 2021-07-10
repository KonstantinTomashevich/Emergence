#pragma once

#include <array>
#include <cstdint>

namespace Emergence::Hashing
{
/// \brief Stateful hasher for byte sequences.
class ByteHasher final
{
public:
    ByteHasher () noexcept;

    ~ByteHasher() noexcept;

    /// \brief Modifies current state by appending given byte.
    void Append (uint8_t _byte) noexcept;

    /// \brief Modifies current state by appending given byte sequence.
    void Append (const uint8_t *_bytes, std::size_t _count) noexcept;

    /// \brief Resets current state to initial values.
    void Clear () noexcept;

    /// \return 64-bit hash value for current state.
    uint64_t GetCurrentValue () const noexcept;

private:
    static constexpr std::size_t DATA_MAX_SIZE = sizeof (uint64_t) * 10u;

    /// \brief Implementation-specific data.
    std::array <uint8_t, DATA_MAX_SIZE> data;
};
} // namespace Emergence::Hashing
