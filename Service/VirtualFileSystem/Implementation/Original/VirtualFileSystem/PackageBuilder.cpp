#include <fstream>

#include <Assert/Assert.hpp>

#include <Container/HashSet.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/PackageFile.hpp>
#include <VirtualFileSystem/PackageBuilder.hpp>
#include <VirtualFileSystem/Reader.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::VirtualFileSystem
{
using namespace Memory::Literals;

struct PackageBuilderEntry
{
    Entry entry;
    Container::Utf8String pathInPackage;
};

struct PackageBuilderImplementationData final
{
    const Context *context;
    Entry output;

    Container::Vector<PackageBuilderEntry> entries {Memory::Profiler::AllocationGroup {
        Memory::Profiler::AllocationGroup {"VirtualFileSystem"_us}, "PackageBuilder"_us}};

    Container::HashSet<Container::Utf8String> registeredPaths {Memory::Profiler::AllocationGroup {
        Memory::Profiler::AllocationGroup {"VirtualFileSystem"_us}, "PackageBuilder"_us}};
};

PackageBuilder::PackageBuilder () noexcept
{
    new (&data) PackageBuilderImplementationData ();
}

PackageBuilder::PackageBuilder (PackageBuilder &&_other) noexcept
{
    new (&data)
        PackageBuilderImplementationData (std::move (block_cast<PackageBuilderImplementationData> (_other.data)));
}

PackageBuilder::~PackageBuilder () noexcept
{
    block_cast<PackageBuilderImplementationData> (data).~PackageBuilderImplementationData ();
}

bool PackageBuilder::Begin (const Context &_context, const Entry &_output) noexcept
{
    if (!_output || _output.GetType () != EntryType::FILE)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to start building package as given output \"",
                       _output ? _output.GetFullPath () : "<invalid>", "\" is not a valid file.");
        return false;
    }

    auto &implementationData = block_cast<PackageBuilderImplementationData> (data);
    EMERGENCE_ASSERT (implementationData.entries.empty ());
    EMERGENCE_ASSERT (implementationData.registeredPaths.empty ());
    EMERGENCE_ASSERT (!implementationData.output);

    implementationData.context = &_context;
    implementationData.output = _output;
    return true;
}

bool PackageBuilder::Add (const Entry &_entry, const Container::Utf8String &_pathInPackage) noexcept
{
    if (!_entry || _entry.GetType () != EntryType::FILE)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to add entry \"",
                       _entry ? _entry.GetFullPath () : "<invalid>", "\" as it is not a valid file.");
        return false;
    }

    auto &implementationData = block_cast<PackageBuilderImplementationData> (data);
    if (implementationData.registeredPaths.contains ({_pathInPackage}))
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to add entry \"",
                       _entry ? _entry.GetFullPath () : "<invalid>", "\" as its path \"", _pathInPackage,
                       "\" is already occupied.");
        return false;
    }

    implementationData.entries.emplace_back (PackageBuilderEntry {_entry, _pathInPackage});
    implementationData.registeredPaths.emplace (_pathInPackage);
    return true;
}

bool PackageBuilder::End () noexcept
{
    auto &implementationData = block_cast<PackageBuilderImplementationData> (data);
    EMERGENCE_ASSERT (implementationData.context);
    EMERGENCE_ASSERT (implementationData.output);
    EMERGENCE_ASSERT (implementationData.output.GetType () == EntryType::FILE);

    auto clean = [&implementationData] ()
    {
        implementationData.entries.clear ();
        implementationData.registeredPaths.clear ();
        implementationData.output = {};
    };

    Original::PackageHeader header;
    std::uint64_t offset = 0u;

    for (const PackageBuilderEntry &entry : implementationData.entries)
    {
        Original::PackageHeaderEntry &headerEntry = header.entries.emplace_back ();
        headerEntry.relativePath = entry.pathInPackage;
        EMERGENCE_ASSERT (entry.entry);
        Reader reader {entry.entry, OpenMode::BINARY};

        if (!reader)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: File \"", entry.entry.GetFullPath (),
                           "\" won't be added to package as it cannot be opened.");
            continue;
        }

        const std::streampos size = reader.InputStream ().seekg (0u, std::ios::end).tellg ();
        if (size == 0u)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: File \"", entry.entry.GetFullPath (),
                           "\" won't be added to package as its size is zero");
            continue;
        }

        headerEntry.size = static_cast<std::uint64_t> (size);
        headerEntry.offset = offset;
        offset += static_cast<std::uint64_t> (size);
    }

    if (header.entries.empty ())
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to build package \"",
                       implementationData.output.GetFullPath (), "\" as there is no entries added to it.");

        clean ();
        return false;
    }

    Writer writer {implementationData.output, OpenMode::BINARY};
    if (!writer)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to build package \"",
                       implementationData.output.GetFullPath (), "\": failed to open file for write.");

        clean ();
        return false;
    }

    Serialization::Binary::SerializeObject (writer.OutputStream (), &header,
                                            Original::PackageHeader::Reflect ().mapping);

    for (const PackageBuilderEntry &entry : implementationData.entries)
    {
        Reader reader {entry.entry, OpenMode::BINARY};
        if (!reader)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: File \"", entry.entry.GetFullPath (),
                           "\" won't be added to package as it cannot be opened.");
            continue;
        }

        const std::size_t size = static_cast<std::size_t> (reader.InputStream ().seekg (0u, std::ios::end).tellg ());
        reader.InputStream ().seekg (0u, std::ios::beg);

        constexpr std::size_t CHUNK_SIZE = 1024u;
        std::size_t read = 0u;
        std::array<char, CHUNK_SIZE> buffer;

        while (read < size)
        {
            const std::size_t toRead = size - read < CHUNK_SIZE ? size - read : CHUNK_SIZE;
            reader.InputStream ().read (buffer.data (), static_cast<std::streamsize> (toRead));

            if (!reader)
            {
                EMERGENCE_LOG (ERROR, "VirtualFileSystem::PackageBuilder: Unable to build package \"",
                               implementationData.output.GetFullPath (), "\": encountered IO error while reading \"",
                               entry.entry.GetFullPath (), "\".");

                clean ();
                return false;
            }

            read += toRead;
            writer.OutputStream ().write (buffer.data (), static_cast<std::streamsize> (toRead));
        }
    }

    clean ();
    return true;
}
} // namespace Emergence::VirtualFileSystem
