#include <Assert/Assert.hpp>

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
    static constexpr std::uint64_t BUFFER_SIZE = 1024u;

    BoundedFileReadBuffer (FILE *_source, std::uint64_t _offset, std::uint64_t _size)
        : file (_source),
          offset (static_cast<long> (_offset)),
          size (static_cast<long> (_size))
    {
    }

    BoundedFileReadBuffer (const BoundedFileReadBuffer &_other) = delete;

    BoundedFileReadBuffer (BoundedFileReadBuffer &&_other) = delete;

    ~BoundedFileReadBuffer () noexcept override
    {
        if (file)
        {
            fclose (file);
        }
    }

    [[nodiscard]] bool IsOpen () const noexcept
    {
        return file;
    }

    EMERGENCE_DELETE_ASSIGNMENT (BoundedFileReadBuffer);

protected:
    int_type underflow () override
    {
        if (ferror (file) != 0)
        {
            setg (buffer, buffer, buffer);
            return traits_type::eof ();
        }

        const auto readOffset = static_cast<std::uint64_t> (gptr () - eback ());
        virtualPosition += readOffset;

        const std::uint64_t realPosition = offset + virtualPosition;
        EMERGENCE_ASSERT (realPosition <= offset + size);

        if (fseek (file, static_cast<long> (realPosition), SEEK_SET) != 0)
        {
            setg (buffer, buffer, buffer);
            return traits_type::eof ();
        }

        EMERGENCE_ASSERT (size >= virtualPosition);
        const std::uint64_t bytesLeft = size - virtualPosition;
        const std::uint64_t toRead = bytesLeft < BUFFER_SIZE ? bytesLeft : BUFFER_SIZE;

        if (toRead == 0)
        {
            setg (buffer, buffer, buffer);
            return traits_type::eof ();
        }

        const auto read = static_cast<std::uint64_t> (fread (buffer, 1u, toRead, file));
        if (read == 0)
        {
            setg (buffer, buffer, buffer);
            return traits_type::eof ();
        }

        setg (buffer, buffer, buffer + read);
        return traits_type::to_int_type (buffer[0]);
    }

    pos_type seekoff (off_type _offset,
                      std::ios::seekdir _direction,
                      std::ios::openmode /*unused*/ = std::ios::in | std::ios::out) override
    {
        const std::uint64_t positionBackup = virtualPosition;
        switch (_direction)
        {
        case std::ios::beg:
        {
            const std::int64_t movedPosition = _offset;
            if (movedPosition >= 0 && movedPosition <= static_cast<std::int64_t> (size))
            {
                virtualPosition = static_cast<std::uint64_t> (movedPosition);
            }
            else
            {
                return traits_type::eof ();
            }

            break;
        }

        case std::ios::cur:
        {
            const std::uint64_t positionIncludingRead = virtualPosition + (gptr () - eback ());
            const std::int64_t movedPosition = static_cast<int64_t> (positionIncludingRead) + _offset;

            if (movedPosition >= 0 && movedPosition <= static_cast<std::int64_t> (size))
            {
                virtualPosition = static_cast<std::uint64_t> (movedPosition);
            }
            else
            {
                return traits_type::eof ();
            }

            break;
        }

        case std::ios::end:
        {
            const std::int64_t movedPosition = static_cast<int64_t> (size) + _offset;
            if (movedPosition >= 0 && movedPosition <= static_cast<std::int64_t> (size))
            {
                virtualPosition = static_cast<std::uint64_t> (movedPosition);
            }
            else
            {
                return traits_type::eof ();
            }

            break;
        }

        // We need default because some implementations define additional "end" enum value.
        default:
            EMERGENCE_ASSERT (false);
        }

        if (virtualPosition != positionBackup)
        {
            // We use simplistic approach here and do not check for overlaps
            // that could technically allow us to preserve buffer.
            setg (buffer, buffer, buffer);
        }

        return static_cast<pos_type> (virtualPosition);
    }

    pos_type seekpos (pos_type _position, std::ios::openmode /*unused*/ = std::ios::in | std::ios::out) override
    {
        setg (buffer, buffer, buffer);
        if (_position < 0)
        {
            return traits_type::eof ();
        }

        if (static_cast<int64_t> (_position) > static_cast<std::int64_t> (size))
        {
            return traits_type::eof ();
        }

        virtualPosition = static_cast<std::uint64_t> (_position);
        return static_cast<pos_type> (virtualPosition);
    }

    int sync () override
    {
        if (ferror (file) != 0)
        {
            return -1;
        }

        setg (buffer, buffer, buffer);
        return 0;
    }

private:
    FILE *file;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::uint64_t virtualPosition = 0;
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
