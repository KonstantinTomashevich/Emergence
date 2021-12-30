#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory
{
// TODO: Common memory usage monitor that allows to differentiate data by unique ids?
//       Ids can be created through Memory::UniqueString usage, for example.

/// \brief Allocator, that manages memory chunks with fixed size. Optimized for allocation and deallocation performance.
class UnorderedPool final
{
public:
    /// \param _groupId Memory allocation group id for profiling.
    /// \param _chunkSize Fixed chunk size.
    /// \invariant _chunkSize must be greater or equal to `sizeof (uintptr_t)`.
    explicit UnorderedPool (UniqueString _groupId, std::size_t _chunkSize) noexcept;

    /// \param _preferredPageCapacity Allocator will create pages with given capacity, if possible.
    /// \see ::UnorderedPool (std::size_t)
    /// \invariant _preferredPageCapacity must be greater than zero.
    UnorderedPool (UniqueString _groupId, std::size_t _chunkSize, std::size_t _preferredPageCapacity) noexcept;

    /// \brief Copying memory pool contradicts with its usage practices.
    UnorderedPool (const UnorderedPool &_other) = delete;

    /// \brief Captures pages of given pool and leaves that pool empty.
    UnorderedPool (UnorderedPool &&_other) noexcept;

    ~UnorderedPool () noexcept;

    /// \brief Acquires memory chunk for new item.
    /// \return Memory chunk or nullptr on failure.
    void *Acquire () noexcept;

    /// \brief Releases given memory chunk.
    /// \invariant Chunk belongs to this pool.
    void Release (void *_chunk) noexcept;

    /// \brief Releases all pages.
    void Clear () noexcept;

    /// \return How much memory pool currently holds?
    [[nodiscard]] std::size_t GetAllocatedSpace () const noexcept;

    /// \brief Copy assigning memory pool contradicts with its usage practices.
    UnorderedPool &operator= (const UnorderedPool &_other) = delete;

    /// \brief Drops all pages from this pool and captures all pages of given pool.
    UnorderedPool &operator= (UnorderedPool &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 6u);
};
} // namespace Emergence::Memory
