#pragma once

#include <array>
#include <cstdint>

namespace Emergence::Memory
{
/// \brief Allocator, that manages memory chunks with fixed size. Based on simple segregated storage idea.
class Pool final
{
public:
    /// \brief Allows const iteration over acquired chunks.
    /// \warning Not all implementations are well-suited for iteration. In some cases iteration could be quite slow.
    class AcquiredChunkConstIterator final
    {
    public:
        ~AcquiredChunkConstIterator () noexcept;

        /// \return Chunk, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        const void *operator * () const noexcept;

        /// \brief Move to next acquired chunk.
        /// \invariant Inside valid bounds, but not in the ending.
        AcquiredChunkConstIterator &operator ++ () noexcept;

        /// \brief Move to next acquired chunk.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        AcquiredChunkConstIterator operator ++ (int) noexcept;

        bool operator == (const AcquiredChunkConstIterator &_other) const noexcept;

        bool operator != (const AcquiredChunkConstIterator &_other) const noexcept;

    private:
        /// Pool constructs iterators.
        friend class Pool;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit AcquiredChunkConstIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over acquired chunks.
    /// \warning Not all implementations are well-suited for iteration. In some cases iteration could be quite slow.
    class AcquiredChunkIterator final
    {
    public:
        ~AcquiredChunkIterator () noexcept;

        /// \return Chunk, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        void *operator * () const noexcept;

        /// \brief Move to next acquired chunk.
        /// \invariant Inside valid bounds, but not in the ending.
        AcquiredChunkIterator &operator ++ () noexcept;

        /// \brief Move to next acquired chunk.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        AcquiredChunkIterator operator ++ (int) noexcept;

        bool operator == (const AcquiredChunkIterator &_other) const noexcept;

        bool operator != (const AcquiredChunkIterator &_other) const noexcept;

    private:
        /// Pool constructs iterators.
        friend class Pool;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit AcquiredChunkIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \param _chunkSize fixed chunk size.
    /// \invariant _chunkSize must be greater or equal to `sizeof (uintptr_t)`.
    explicit Pool (std::size_t _chunkSize) noexcept;

    /// \param _preferredPageCapacity allocator will create pages with given capacity, if possible.
    /// \see ::Pool (std::size_t)
    /// \invariant _preferredPageCapacity must be greater than zero.
    Pool (std::size_t _chunkSize, std::size_t _preferredPageCapacity) noexcept;

    /// \brief Copying memory pool contradicts with its usage practices.
    Pool (const Pool &_other) = delete;

    // \brief Captures pages of given pool and leaves that pool empty.
    Pool (Pool &&_other) noexcept;

    ~Pool () noexcept;

    /// \brief Acquires memory chunk for new item.
    /// \return Memory chunk or nullptr on failure.
    void *Acquire () noexcept;

    /// \brief Releases given memory chunk.
    /// \invariant Chunk belongs to this pool.
    /// \warning Invalidates iterator, that points to this record.
    void Release (void *_chunk) noexcept;

    /// \brief Releases all empty pages.
    void Shrink () noexcept;

    /// \brief Releases all pages.
    /// \warning Invalidates iterators.
    void Clear () noexcept;

    /// \return How much memory pool currently holds?
    std::size_t GetAllocatedSpace () const noexcept;

    /// \return Iterator, that points to beginning of acquired chunks sequence.
    AcquiredChunkConstIterator BeginAcquired () const noexcept;

    /// \return Iterator, that points to ending of acquired chunks sequence.
    AcquiredChunkConstIterator EndAcquired () const noexcept;

    /// \return Iterator, that points to beginning of acquired chunks sequence.
    AcquiredChunkIterator BeginAcquired () noexcept;

    /// \return Iterator, that points to ending of acquired chunks sequence.
    AcquiredChunkIterator EndAcquired () noexcept;

    /// \brief Copy assigning memory pool contradicts with its usage practices.
    Pool &operator = (const Pool &_other) = delete;

    /// \brief Drops all pages from this pool and captures all pages of given pool.
    Pool &operator = (Pool &&_other) noexcept;

private:
    /// \brief Max size of pool implementation object.
    ///
    /// \details It could be critical for performance to store pool object, not pool handle, as field of other class.
    /// Therefore we specify max object size for any implementation and reserve this space using std::array.
    static constexpr std::size_t MAX_DATA_SIZE = sizeof (uintptr_t) * 5u;

    /// \brief Stub, that reserves space for implementation object.
    /// \see ::MAX_FIELDS_SIZE.
    std::array <uint8_t, MAX_DATA_SIZE> data;
};

/// \brief Wraps Pool::BeginAcquired for foreach sentences.
Pool::AcquiredChunkConstIterator begin (const Pool &_pool) noexcept;

/// \brief Wraps Pool::EndAcquired for foreach sentences.
Pool::AcquiredChunkConstIterator end (const Pool &_pool) noexcept;

/// \brief Wraps Pool::BeginAcquired for foreach sentences.
Pool::AcquiredChunkIterator begin (Pool &_pool) noexcept;

/// \brief Wraps Pool::EndAcquired for foreach sentences.
Pool::AcquiredChunkIterator end (Pool &_pool) noexcept;
} // namespace Emergence::Memory