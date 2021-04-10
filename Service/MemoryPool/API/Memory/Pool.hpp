#pragma once

#include <array>
#include <cstdint>

namespace Emergence::Memory
{
/// \brief Allocator, that manages memory chunks with fixed size. Based on simple segregated storage idea.
class Pool final
{
public:
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
    /// \return memory chunk or nullptr on failure.
    void *Acquire () noexcept;

    /// \brief Releases given memory chunk.
    /// \invariant chunk belongs to this pool.
    void Release (void *chunk) noexcept;

    /// \brief Releases all empty pages.
    void Shrink () noexcept;

    /// \brief Releases all pages.
    void Clear () noexcept;

private:
    /// \brief Max total size of pool fields in bytes.
    ///
    /// \details It could be critical for performance to store pool object, not pool handle, as field of other class.
    /// Therefore we specify max total size of fields for any implementation and reserve this space using std::array.
    /// Currently max size of pool object is equal to size of boost::pool.
    static constexpr std::size_t MAX_FIELDS_SIZE = sizeof (uintptr_t) * 7u;

    /// \brief Stub, that reserves space for implementation fields.
    /// \see ::MAX_FIELDS_SIZE.
    std::array <uint8_t, MAX_FIELDS_SIZE> fields;
};
} // namespace Emergence::Memory