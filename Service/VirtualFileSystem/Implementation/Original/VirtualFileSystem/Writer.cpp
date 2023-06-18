#include <fstream>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/Wrappers.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::VirtualFileSystem
{
Writer::Writer (const Entry &_entry, OpenMode _openMode) noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (_entry.data);
    Original::FileWriteContext context = entryData.owner->OpenFileForWrite (entryData.object, _openMode);
    new (&data) std::ofstream {context.file};
}

Writer::~Writer () noexcept
{
    auto &stream = block_cast<std::ofstream> (data);
    if (stream.is_open ())
    {
        stream.close ();
    }

    stream.~basic_ofstream ();
}

bool Writer::IsValid () const noexcept
{
    return block_cast<std::ofstream> (data).is_open () && !block_cast<std::ofstream> (data).fail ();
}

std::ostream &Writer::OutputStream () noexcept
{
    return block_cast<std::ofstream> (data);
}
} // namespace Emergence::VirtualFileSystem
