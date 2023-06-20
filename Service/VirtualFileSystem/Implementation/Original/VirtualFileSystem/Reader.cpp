#include <fstream>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/Wrappers.hpp>
#include <VirtualFileSystem/Reader.hpp>

namespace Emergence::VirtualFileSystem
{
class BoundedFileReadBuffer final : public std::streambuf
{
public:
    static constexpr std::size_t BUFFER_SIZE = 1024;

    BoundedFileReadBuffer (FILE *_source, std::uint64_t _offset, std::uint64_t _size)
        : file (_source),
          offset (static_cast<long> (_offset)),
          size (static_cast<long> (_size))
    {
        seekoff (0, std::ios::beg);
    }

    BoundedFileReadBuffer (const BoundedFileReadBuffer &_other) = delete;

    BoundedFileReadBuffer (BoundedFileReadBuffer &&_other) = delete;

    ~BoundedFileReadBuffer () noexcept
    {
        if (file)
        {
            fclose (file);
        }
    }

    bool IsOpen () const noexcept
    {
        return file;
    }

    EMERGENCE_DELETE_ASSIGNMENT (BoundedFileReadBuffer);

protected:
    int_type underflow () override
    {
        if (ferror (file) != 0)
        {
            return -1;
        }

        const long chunkBegin = ftell (file);
        const long readOffset = static_cast<long> (gptr () - eback ());

        if (fseek (file, chunkBegin + readOffset, SEEK_SET) != 0)
        {
            return EOF;
        }

        const long newChunkBegin = ftell (file);
        const long bytesLeft = offset + size - newChunkBegin;
        const long toRead = bytesLeft < static_cast<long> (BUFFER_SIZE) ? bytesLeft : static_cast<long> (BUFFER_SIZE);

        if (toRead == 0)
        {
            return EOF;
        }

        const std::size_t read = fread (buffer, 1u, toRead, file);
        if (read == 0)
        {
            return EOF;
        }

        setg (buffer, buffer, buffer + read);
        if (fseek (file, newChunkBegin, SEEK_SET) != 0)
        {
            return EOF;
        }

        return *eback ();
    }

    pos_type seekoff (off_type _offset,
                      std::ios::seekdir _direction,
                      std::ios::openmode /*unused*/ = std::ios::in | std::ios::out) override
    {
        if (ferror (file) != 0)
        {
            return -1;
        }

        switch (_direction)
        {
        case std::ios::beg:
        {
            if (_offset < 0)
            {
                return -1;
            }

            if (_offset > size)
            {
                return -1;
            }

            if (fseek (file, static_cast<long> (offset + _offset), SEEK_SET) != 0)
            {
                return -1;
            }

            break;
        }

        case std::ios::cur:
        {
            const pos_type current = ftell (file) + (gptr () - eback ());
            if (current + _offset < offset)
            {
                return -1;
            }

            if (current + _offset > offset + size)
            {
                return -1;
            }

            if (fseek (file, static_cast<long> (current + _offset), SEEK_SET) != 0)
            {
                return -1;
            }

            break;
        }

        case std::ios::end:
        {
            if (_offset > 0)
            {
                return -1;
            }

            if (-_offset > size)
            {
                return -1;
            }

            if (fseek (file, static_cast<long> (offset + size + _offset), SEEK_SET) != 0)
            {
                return -1;
            }

            break;
        }
        }

        if (_offset != 0 || _direction != std::ios::cur)
        {
            setg (buffer, buffer, buffer);
        }

        return ftell (file) - offset + (gptr () - eback ());
    }

    pos_type seekpos (pos_type _position, std::ios::openmode /*unused*/ = std::ios::in | std::ios::out) override
    {
        if (ferror (file) != 0)
        {
            return -1;
        }

        if (_position < 0)
        {
            return -1;
        }

        if (_position > size)
        {
            return -1;
        }

        fpos_t position = offset + _position;
        if (fsetpos (file, &position) != 0)
        {
            return -1;
        }

        return _position;
    }

    int sync () override
    {
        if (ferror (file) != 0)
        {
            return -1;
        }

        underflow ();
        return 0;
    }

private:
    FILE *file;
    long offset = 0;
    long size = 0;
    char buffer[BUFFER_SIZE];
};

struct ReaderImplementationData
{
    ReaderImplementationData (FILE *_source, std::uint64_t _offset, std::uint64_t _size) noexcept
        : buffer (_source, _offset, _size),
          input (&buffer)
    {
    }

    BoundedFileReadBuffer buffer;
    std::istream input;
};

Reader::Reader (const Entry &_entry, OpenMode _openMode) noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (_entry.data);
    Original::FileReadContext context = entryData.owner->OpenFileForRead (entryData.object, _openMode);
    new (&data) ReaderImplementationData {context.file, context.offset, context.size};
}

Reader::~Reader () noexcept
{
    block_cast<ReaderImplementationData> (data).~ReaderImplementationData ();
}

bool Reader::IsValid () const noexcept
{
    return block_cast<ReaderImplementationData> (data).buffer.IsOpen () &&
           !block_cast<ReaderImplementationData> (data).input.fail ();
}

std::istream &Reader::InputStream () noexcept
{
    return block_cast<ReaderImplementationData> (data).input;
}
} // namespace Emergence::VirtualFileSystem
