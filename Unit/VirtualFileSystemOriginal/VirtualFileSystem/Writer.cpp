#include <fstream>

#include <Assert/Assert.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/Wrappers.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::VirtualFileSystem
{
class CFileWriteBuffer final : public std::streambuf
{
public:
    static constexpr std::uint64_t BUFFER_SIZE = 1024u;

    CFileWriteBuffer (FILE *_file)
        : file (_file)
    {
        setp (buffer, buffer + BUFFER_SIZE);
    }

    CFileWriteBuffer (const CFileWriteBuffer &_other) = delete;

    CFileWriteBuffer (CFileWriteBuffer &&_other) = delete;

    ~CFileWriteBuffer () noexcept override
    {
        if (pptr () != pbase ())
        {
            overflow (traits_type::eof ());
        }

        if (file)
        {
            fclose (file);
        }
    }

    [[nodiscard]] bool IsOpen () const noexcept
    {
        return file;
    }

    EMERGENCE_DELETE_ASSIGNMENT (CFileWriteBuffer);

protected:
    int_type overflow (int_type _character) override
    {
        if (ferror (file))
        {
            setp (buffer, buffer + BUFFER_SIZE);
            return traits_type::eof ();
        }

        const auto toWrite = static_cast<std::uint64_t> (pptr () - pbase ());
        if (toWrite > 0u)
        {
            const auto written =
                static_cast<std::uint64_t> (fwrite (buffer, 1u, static_cast<std::size_t> (toWrite), file));

            if (toWrite != written)
            {
                setp (buffer, buffer + BUFFER_SIZE);
                return traits_type::eof ();
            }
        }

        if (_character != traits_type::eof ())
        {
            if (fputc (_character, file) == traits_type::eof ())
            {
                setp (buffer, buffer + BUFFER_SIZE);
                return traits_type::eof ();
            }
        }

        setp (buffer, buffer + BUFFER_SIZE);
        return 0;
    }

    pos_type seekoff (off_type _offset, std::ios_base::seekdir _direction, std::ios_base::openmode /*unused*/) override
    {
        if (pbase () != pptr ())
        {
            if (overflow (traits_type::eof ()) == traits_type::eof ())
            {
                return traits_type::eof ();
            }
        }

        switch (_direction)
        {
        case std::ios::beg:
            if (fseek (file, static_cast<long> (_offset), SEEK_SET) != 0)
            {
                return traits_type::eof ();
            }

            break;

        case std::ios::cur:
            if (fseek (file, static_cast<long> (_offset), SEEK_CUR) != 0)
            {
                return traits_type::eof ();
            }

            break;

        case std::ios::end:
            if (fseek (file, static_cast<long> (_offset), SEEK_END) != 0)
            {
                return traits_type::eof ();
            }

            break;

        // We need default because some implementations define additional "end" enum value.
        default:
            EMERGENCE_ASSERT (false);
        }

        return static_cast<pos_type> (ftell (file));
    }

    pos_type seekpos (pos_type _position, std::ios_base::openmode /*unused*/) override
    {
        if (pbase () != pptr ())
        {
            if (overflow (traits_type::eof ()) == traits_type::eof ())
            {
                return traits_type::eof ();
            }
        }

        if (fseek (file, static_cast<long> (_position), SEEK_SET) != 0)
        {
            return traits_type::eof ();
        }

        return static_cast<pos_type> (ftell (file));
    }

    int sync () override
    {
        if (pbase () != pptr ())
        {
            if (overflow (traits_type::eof ()) == traits_type::eof ())
            {
                return traits_type::eof ();
            }
        }

        return basic_streambuf::sync ();
    }

private:
    FILE *file = nullptr;
    char buffer[BUFFER_SIZE];
};

struct ImplementationData final
{
    ImplementationData (FILE *_file) noexcept
        : file (_file),
          buffer (_file),
          output (&buffer)
    {
    }

    FILE *file = nullptr;
    CFileWriteBuffer buffer;
    std::ostream output;
};

Writer::Writer (const Entry &_entry) noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (_entry.data);
    Original::FileWriteContext context = entryData.owner->OpenFileForWrite (entryData.object);
    new (&data) ImplementationData {context.file};
}

Writer::~Writer () noexcept
{
    block_cast<ImplementationData> (data).~ImplementationData ();
}

bool Writer::IsValid () const noexcept
{
    return block_cast<ImplementationData> (data).buffer.IsOpen () &&
           !block_cast<ImplementationData> (data).output.fail ();
}

std::ostream &Writer::OutputStream () noexcept
{
    return block_cast<ImplementationData> (data).output;
}
} // namespace Emergence::VirtualFileSystem
