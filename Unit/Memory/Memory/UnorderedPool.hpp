#pragma once

#include <MemoryApi.hpp>

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory
{
/// \brief Allocator, that manages memory chunks with fixed size. Optimized for allocation and deallocation performance.
class MemoryApi UnorderedPool final
{
public:
    /// \param _chunkSize Fixed chunk size.
    /// \param _alignment Address alignment, required for each chunk.
    /// \invariant _chunkSize must be greater or equal to `sizeof (std::uintptr_t)`.
    UnorderedPool (Profiler::AllocationGroup _group, std::size_t _chunkSize, std::size_t _alignment) noexcept;

    /// \param _preferredPageCapacity Allocator will create pages with given capacity, if possible.
    /// \see ::UnorderedPool (std::size_t)
    /// \invariant _preferredPageCapacity must be greater than zero.
    UnorderedPool (Profiler::AllocationGroup _group,
                   std::size_t _chunkSize,
                   std::size_t _alignment,
                   std::size_t _preferredPageCapacity) noexcept;

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

    /// \return True if there is no acquired items.
    [[nodiscard]] bool IsEmpty () const noexcept;

    /// \return Allocation group to which this allocator belongs.
    /// \warning Group will report zero memory usage if it is a placeholder or
    ///          if executable is linked to no-profile implementation.
    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept;

    /// \brief Copy assigning memory pool contradicts with its usage practices.
    UnorderedPool &operator= (const UnorderedPool &_other) = delete;

    /// \brief Drops all pages from this pool and captures all pages of given pool.
    UnorderedPool &operator= (UnorderedPool &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 7u);
};
} // namespace Emergence::Memory
