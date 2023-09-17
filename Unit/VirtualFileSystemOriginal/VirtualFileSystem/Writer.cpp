#include <fstream>

#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

#include <Container/Variant.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/VirtualFileBuffer.hpp>
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

struct WriterImplementationData final
{
    WriterImplementationData (FILE *_file) noexcept
        : buffer (std::in_place_type<CFileWriteBuffer>, _file),
          output (&std::get<CFileWriteBuffer> (buffer))
    {
    }

    WriterImplementationData (Original::VirtualFileData *_file) noexcept
        : buffer (std::in_place_type<Original::VirtualFileWriteBuffer>, _file),
          output (&std::get<Original::VirtualFileWriteBuffer> (buffer))
    {
    }

    [[nodiscard]] bool IsOpen () const noexcept
    {
        return std::visit (
            [] (auto &_buffer)
            {
                return _buffer.IsOpen ();
            },
            buffer);
    }

    Container::Variant<CFileWriteBuffer, Original::VirtualFileWriteBuffer> buffer;
    std::ostream output;
};

Writer::Writer (const Entry &_entry) noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (_entry.data);
    Original::FileWriteContext context = entryData.owner->OpenFileForWrite (entryData.object);

    switch (context.type)
    {
    case Original::FileIOContextType::REAL_FILE:
        new (&data) WriterImplementationData {context.realFile};
        break;

    case Original::FileIOContextType::VIRTUAL_FILE:
        new (&data) WriterImplementationData {context.virtualFile};
        break;
    }
}

Writer::~Writer () noexcept
{
    block_cast<WriterImplementationData> (data).~WriterImplementationData ();
}

bool Writer::IsValid () const noexcept
{
    return block_cast<WriterImplementationData> (data).IsOpen () &&
           !block_cast<WriterImplementationData> (data).output.fail ();
}

std::ostream &Writer::OutputStream () noexcept
{
    return block_cast<WriterImplementationData> (data).output;
}
} // namespace Emergence::VirtualFileSystem
