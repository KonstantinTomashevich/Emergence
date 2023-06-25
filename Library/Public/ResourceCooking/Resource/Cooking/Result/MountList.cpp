#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Result/MountList.hpp>

#include <Serialization/Binary.hpp>

#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::Resource::Cooking
{
bool ProduceMountList (Context &_context,
                       const std::string_view &_groupName,
                       const VirtualFileSystem::MountConfigurationList &_list) noexcept
{
    EMERGENCE_LOG (INFO, "Resource::Cooking: Producing mount list for group \"", _groupName, "\".");
    const Container::Utf8String outputName {EMERGENCE_BUILD_STRING ("Mount", _groupName, ".bin")};
    VirtualFileSystem::Entry outputEntry {_context.GetFinalResultDirectory (), outputName};

    if (!outputEntry)
    {
        outputEntry = _context.GetVirtualFileSystem ().CreateFile (_context.GetFinalResultDirectory (), outputName);
        if (!outputEntry)
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to create file for mount list \"", _groupName,
                           "\" output.");
            return false;
        }
    }

    VirtualFileSystem::Writer writer {outputEntry, VirtualFileSystem::OpenMode::BINARY};
    if (!writer)
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to open \"", outputEntry.GetFullPath (), "\" for write.");
        return false;
    }

    Serialization::Binary::SerializeObject (writer.OutputStream (), &_list,
                                            VirtualFileSystem::MountConfigurationList::Reflect ().mapping);
    EMERGENCE_LOG (INFO, "Resource::Cooking: Successfully produced mount list for group \"", _groupName, "\".");
    return true;
}
} // namespace Emergence::Resource::Cooking
