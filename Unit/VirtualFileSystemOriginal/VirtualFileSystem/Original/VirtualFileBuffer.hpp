#pragma once

#include <VirtualFileSystem/Original/Core.hpp>

namespace Emergence::VirtualFileSystem::Original
{
class VirtualFileBufferBase : public std::streambuf
{
public:
    VirtualFileBufferBase (const VirtualFileBufferBase &_other) = delete;

    VirtualFileBufferBase (VirtualFileBufferBase &&_other) = delete;

    ~VirtualFileBufferBase () noexcept override = default;

    EMERGENCE_DELETE_ASSIGNMENT (VirtualFileBufferBase);

    [[nodiscard]] bool IsOpen () const noexcept
    {
        return file;
    }

protected:
    VirtualFileBufferBase (Original::VirtualFileData *_file) noexcept;

    pos_type seekoff (off_type _offset, std::ios::seekdir _direction, std::ios::openmode _openMode) override;

    pos_type seekpos (pos_type _position, std::ios::openmode _openMode) override;

    bool SeekOffsetFromCurrent (std::int64_t _offset) noexcept;

    VirtualFileData *file = nullptr;
    VirtualFileChunk *currentChunk = nullptr;
    std::uint64_t localPosition = 0u;
    std::uint64_t globalPosition = 0u;
};

class VirtualFileReadBuffer final : public VirtualFileBufferBase
{
public:
    VirtualFileReadBuffer (Original::VirtualFileData *_file) noexcept;

protected:
    int_type underflow () override;
};

class VirtualFileWriteBuffer final : public VirtualFileBufferBase
{
public:
    static constexpr std::uint64_t FIRST_CHUNK_SIZE = 8192u;

    static constexpr std::uint64_t MAX_CHUNK_SIZE = 1073741824u;

    static constexpr std::uint64_t CHUNK_SIZE_MULTIPLIER = 4u;

    VirtualFileWriteBuffer (Original::VirtualFileData *_file) noexcept;

    VirtualFileWriteBuffer (const VirtualFileWriteBuffer &_other) = delete;

    VirtualFileWriteBuffer (VirtualFileWriteBuffer &&_other) = delete;

    ~VirtualFileWriteBuffer () noexcept override;

    EMERGENCE_DELETE_ASSIGNMENT (VirtualFileWriteBuffer);

protected:
    int_type overflow (int_type _character) override;
};
} // namespace Emergence::VirtualFileSystem::Original
