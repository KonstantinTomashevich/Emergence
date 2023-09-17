#include <Assert/Assert.hpp>

#include <VirtualFileSystem/Original/VirtualFileBuffer.hpp>

namespace Emergence::VirtualFileSystem::Original
{
VirtualFileBufferBase::VirtualFileBufferBase (Original::VirtualFileData *_file) noexcept
    : file (_file),
      currentChunk (_file->firstChunk)
{
    // Validate file structure.
#if defined(EMERGENCE_ASSERT_ENABLED)
    Original::VirtualFileChunk *chunk = file->firstChunk;
    EMERGENCE_ASSERT (!chunk || !chunk->previous);

    while (chunk)
    {
        EMERGENCE_ASSERT (chunk->used == chunk->size || !chunk->next);
        Original::VirtualFileChunk *next = chunk->next;
        EMERGENCE_ASSERT (!next || chunk == next->previous);
        chunk = next;
    }
#endif
}

VirtualFileBufferBase::pos_type VirtualFileBufferBase::seekoff (VirtualFileReadBuffer::off_type _offset,
                                                                std::ios::seekdir _direction,
                                                                std::ios::openmode _openMode)
{
    EMERGENCE_ASSERT (_openMode == std::ios::in || _openMode == std::ios::out);
    if (_openMode == std::ios::out && pbase () != pptr ())
    {
        if (overflow (traits_type::eof ()) == traits_type::eof ())
        {
            return traits_type::eof ();
        }
    }

    const std::uint64_t positionBackup = globalPosition;
    switch (_direction)
    {
    case std::ios::beg:
    {
        globalPosition = 0u;
        localPosition = 0u;
        currentChunk = file->firstChunk;

        if (!SeekOffsetFromCurrent (static_cast<std::int64_t> (_offset)))
        {
            return traits_type::eof ();
        }

        break;
    }

    case std::ios::cur:
    {
        const std::int64_t operationOffset =
            _openMode == std::ios::in ? static_cast<std::int64_t> (gptr () - eback ()) : 0;

        if (!SeekOffsetFromCurrent (static_cast<std::int64_t> (_offset) + operationOffset))
        {
            return traits_type::eof ();
        }

        break;
    }

    case std::ios::end:
    {
        currentChunk = file->lastChunk;
        globalPosition = static_cast<std::uint64_t> (file->size);
        localPosition = currentChunk->used;

        if (!SeekOffsetFromCurrent (static_cast<std::int64_t> (_offset)))
        {
            return traits_type::eof ();
        }

        break;
    }

    // We need default because some implementations define additional "end" enum value.
    default:
        EMERGENCE_ASSERT (false);
    }

    if (globalPosition != positionBackup)
    {
        // We use simplistic approach here and do not check for overlaps
        // that could technically allow us to preserve buffer.
        if (_openMode == std::ios::in)
        {
            setg (nullptr, nullptr, nullptr);
        }
        else
        {
            setp (nullptr, nullptr);
        }
    }

    // NOLINTNEXTLINE(*-narrowing-conversions): values here should be low enough to avoid errors.
    return static_cast<pos_type> (globalPosition);
}

VirtualFileBufferBase::pos_type VirtualFileBufferBase::seekpos (VirtualFileReadBuffer::pos_type _position,
                                                                std::ios::openmode _openMode)
{
    if (_position < 0)
    {
        return traits_type::eof ();
    }

    return seekoff (static_cast<off_type> (_position), std::ios::beg, _openMode);
}

bool VirtualFileBufferBase::SeekOffsetFromCurrent (std::int64_t _offset) noexcept
{
    if (!currentChunk)
    {
        return false;
    }

    if (_offset >= 0)
    {
        while (true)
        {
            const auto leftInChunk = static_cast<std::int64_t> (currentChunk->used - localPosition);
            if (_offset >= leftInChunk)
            {
                globalPosition += static_cast<std::uint64_t> (leftInChunk);
                _offset -= leftInChunk;

                if (currentChunk->next)
                {
                    currentChunk = currentChunk->next;
                    localPosition = 0u;
                }
                else
                {
                    localPosition = currentChunk->used;
                    break;
                }
            }
            else
            {
                localPosition += static_cast<std::uint64_t> (_offset);
                globalPosition += localPosition;
                _offset = 0;
                break;
            }
        }
    }
    else
    {
        _offset = -_offset;
        while (true)
        {
            const auto leftInChunk = static_cast<std::int64_t> (localPosition);
            if (_offset > leftInChunk)
            {
                globalPosition -= static_cast<std::uint64_t> (leftInChunk);
                _offset -= leftInChunk;

                if (currentChunk->previous)
                {
                    currentChunk = currentChunk->previous;
                    localPosition = currentChunk->used;
                }
                else
                {
                    localPosition = 0u;
                    break;
                }
            }
            else
            {
                localPosition -= static_cast<std::uint64_t> (_offset);
                globalPosition -= static_cast<std::uint64_t> (_offset);
                _offset = 0;
                break;
            }
        }
    }

    return _offset == 0 && currentChunk && localPosition <= currentChunk->used;
}

VirtualFileReadBuffer::VirtualFileReadBuffer (Original::VirtualFileData *_file) noexcept
    : VirtualFileBufferBase (_file)
{
}

VirtualFileReadBuffer::int_type VirtualFileReadBuffer::underflow ()
{
    if (!file || !currentChunk)
    {
        setg (nullptr, nullptr, nullptr);
        return traits_type::eof ();
    }

    if (!SeekOffsetFromCurrent (static_cast<std::int64_t> (gptr () - eback ())))
    {
        setg (nullptr, nullptr, nullptr);
        return traits_type::eof ();
    }

    const std::uint64_t left = currentChunk->used - localPosition;
    if (left == 0u)
    {
        setg (nullptr, nullptr, nullptr);
        return traits_type::eof ();
    }

    auto *begin = reinterpret_cast<char *> (&currentChunk->data[localPosition]);
    char *end = begin + left;
    setg (begin, begin, end);
    return traits_type::to_int_type (*begin);
}

VirtualFileWriteBuffer::VirtualFileWriteBuffer (Original::VirtualFileData *_file) noexcept
    : VirtualFileBufferBase (_file)
{
    // Not the most optimal, because we can theoretically reuse memory
    // of the old file, but we don't do that for now as simplification.
    file->Reset ();

    currentChunk =
        new (file->chunkHeap->Acquire (sizeof (VirtualFileChunk) + FIRST_CHUNK_SIZE, alignof (VirtualFileChunk)))
            VirtualFileChunk {};
    currentChunk->size = FIRST_CHUNK_SIZE;

    file->firstChunk = currentChunk;
    file->lastChunk = currentChunk;
    file->lastWriteTime = std::chrono::file_clock ::now ();
}

VirtualFileWriteBuffer::~VirtualFileWriteBuffer () noexcept
{
    if (pptr () != pbase ())
    {
        overflow (traits_type::eof ());
    }

    if (file && file->lastChunk && file->lastChunk->used == 0u)
    {
        VirtualFileChunk *previous = file->lastChunk;
        file->chunkHeap->Release (file->lastChunk, sizeof (VirtualFileChunk) + file->lastChunk->size);

        if (file->lastChunk == file->firstChunk)
        {
            file->firstChunk = previous;
            EMERGENCE_ASSERT (!previous);
        }

        if (previous)
        {
            previous->next = nullptr;
        }

        file->lastChunk = previous;
        EMERGENCE_ASSERT (!previous || previous->used == previous->size);
    }
}

VirtualFileWriteBuffer::int_type VirtualFileWriteBuffer::overflow (VirtualFileWriteBuffer::int_type _character)
{
    if (!file || !currentChunk)
    {
        setp (nullptr, nullptr);
        return traits_type::eof ();
    }

    const auto toWrite = static_cast<std::uint64_t> (pptr () - pbase ());
    if (toWrite > 0u)
    {
        localPosition += toWrite;
        globalPosition += toWrite;
        EMERGENCE_ASSERT (localPosition <= currentChunk->size);

        if (localPosition > currentChunk->used)
        {
            file->size += localPosition - currentChunk->used;
            currentChunk->used = localPosition;
        }

        if (localPosition == currentChunk->size)
        {
            if (!currentChunk->next)
            {
                const std::uint64_t newChunkSize =
                    std::min (currentChunk->size * CHUNK_SIZE_MULTIPLIER, MAX_CHUNK_SIZE);

                auto *newChunk = new (file->chunkHeap->Acquire (sizeof (VirtualFileChunk) + newChunkSize,
                                                                alignof (VirtualFileChunk))) VirtualFileChunk {};

                newChunk->previous = currentChunk;
                newChunk->size = newChunkSize;
                currentChunk->next = newChunk;
                file->lastChunk = newChunk;
            }

            currentChunk = currentChunk->next;
            localPosition = 0u;
        }
    }

    char *begin = reinterpret_cast<char *> (&currentChunk->data[localPosition]);
    char *end = reinterpret_cast<char *> (&currentChunk->data[currentChunk->size]);
    EMERGENCE_ASSERT (begin != end);
    setp (begin, end);

    if (_character != traits_type::eof ())
    {
        *begin = traits_type::to_char_type (_character);
        pbump (1);

        if (pptr () == epptr ())
        {
            return overflow (traits_type::eof ());
        }
    }

    file->lastWriteTime = std::chrono::file_clock ::now ();
    return 0;
}
} // namespace Emergence::VirtualFileSystem::Original
