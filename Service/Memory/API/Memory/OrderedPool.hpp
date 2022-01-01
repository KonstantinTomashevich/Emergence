#pragma once

#include <array>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory
{
/// \brief Allocator, that manages memory chunks with fixed size.
/// \details Ensures that newly acquired chunk always has lowest possible address. This strategy allows to provide
///          acquired chunk iteration and shrink operation with reasonable performance. Also it slightly increases
///          cache coherence.
/// \warning Allocation and deallocation operations are rather slow due to the need to maintain ordering.
///          If you need to actively allocate and dealocate objects, use UnorderedPool.
class OrderedPool final
{
public:
    /// \brief Allows const iteration over acquired chunks.
    class AcquiredChunkConstIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredChunkConstIterator, const void *);

    private:
        /// Pool constructs iterators.
        friend class OrderedPool;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 4u);

        explicit AcquiredChunkConstIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows iteration over acquired chunks.
    class AcquiredChunkIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredChunkIterator, void *);

    private:
        /// Pool constructs iterators.
        friend class OrderedPool;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 4u);

        explicit AcquiredChunkIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \param _groupId Memory allocation group id for profiling.
    /// \param _chunkSize Fixed chunk size.
    /// \invariant _chunkSize must be greater or equal to `sizeof (uintptr_t)`.
    explicit OrderedPool (UniqueString _groupId, std::size_t _chunkSize) noexcept;

    /// \param _preferredPageCapacity allocator will create pages with given capacity, if possible.
    /// \see ::Pool (std::size_t)
    /// \invariant _preferredPageCapacity must be greater than zero.
    OrderedPool (UniqueString _groupId, std::size_t _chunkSize, std::size_t _preferredPageCapacity) noexcept;

    /// Copying memory pool contradicts with its usage practices.
    OrderedPool (const OrderedPool &_other) = delete;

    /// \brief Captures pages of given pool and leaves that pool empty.
    OrderedPool (OrderedPool &&_other) noexcept;

    ~OrderedPool () noexcept;

    /// \brief Acquires memory chunk for new item.
    /// \return Memory chunk or nullptr on failure.
    /// \warning Invalidates iterators.
    void *Acquire () noexcept;

    /// \brief Releases given memory chunk.
    /// \invariant Chunk belongs to this pool.
    /// \warning Invalidates iterator, that points to released chunk.
    void Release (void *_chunk) noexcept;

    /// \brief Releases all empty pages.
    /// \warning Invalidates iterators.
    void Shrink () noexcept;

    /// \brief Releases all pages.
    /// \warning Invalidates iterators.
    void Clear () noexcept;

    /// \return How much memory pool currently holds?
    [[nodiscard]] std::size_t GetAllocatedSpace () const noexcept;

    /// \return Iterator, that points to beginning of acquired chunks sequence.
    [[nodiscard]] AcquiredChunkConstIterator BeginAcquired () const noexcept;

    /// \return Iterator, that points to ending of acquired chunks sequence.
    [[nodiscard]] AcquiredChunkConstIterator EndAcquired () const noexcept;

    /// \return Iterator, that points to beginning of acquired chunks sequence.
    AcquiredChunkIterator BeginAcquired () noexcept;

    /// \return Iterator, that points to ending of acquired chunks sequence.
    AcquiredChunkIterator EndAcquired () noexcept;

    /// \brief Copy assigning memory pool contradicts with its usage practices.
    OrderedPool &operator= (const OrderedPool &_other) = delete;

    /// \brief Drops all pages from this pool and captures all pages of given pool.
    OrderedPool &operator= (OrderedPool &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 6u);
};

/// \brief Wraps Pool::BeginAcquired for foreach sentences.
OrderedPool::AcquiredChunkConstIterator begin (const OrderedPool &_pool) noexcept;

/// \brief Wraps Pool::EndAcquired for foreach sentences.
OrderedPool::AcquiredChunkConstIterator end (const OrderedPool &_pool) noexcept;

/// \brief Wraps Pool::BeginAcquired for foreach sentences.
OrderedPool::AcquiredChunkIterator begin (OrderedPool &_pool) noexcept;

/// \brief Wraps Pool::EndAcquired for foreach sentences.
OrderedPool::AcquiredChunkIterator end (OrderedPool &_pool) noexcept;
} // namespace Emergence::Memory
