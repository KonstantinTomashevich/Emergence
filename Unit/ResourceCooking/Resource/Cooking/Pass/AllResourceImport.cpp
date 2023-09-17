#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Pass/AllResourceImport.hpp>

namespace Emergence::Resource::Cooking
{
bool AllResourceImportPass (Context &_context) noexcept
{
    EMERGENCE_LOG (INFO, "Resource::Cooking: All resource import pass started.");
    for (const auto &[typeName, type] : _context.GetInitialResourceProvider ().GetObjectTypesRegistry ().GetRegistry ())
    {
        for (auto cursor = _context.GetInitialResourceProvider ().FindObjectsByType (type); **cursor; ++cursor)
        {
            const Memory::UniqueString id = *cursor;
            const VirtualFileSystem::Entry entry = _context.GetInitialResourceProvider ().GetObjectEntry (type, id);

            if (entry.GetType () != VirtualFileSystem::EntryType::FILE)
            {
                EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to get virtual file system entry for \"", id, "\".");
                return false;
            }

            const Provider::ObjectFormat format = _context.GetInitialResourceProvider ().GetObjectFormat (type, id);
            _context.GetResourceList ().AddObject ({id, type, entry, format});
        }
    }

    for (auto cursor = _context.GetInitialResourceProvider ().VisitAllThirdParty (); **cursor; ++cursor)
    {
        const Memory::UniqueString id = *cursor;
        const VirtualFileSystem::Entry entry = _context.GetInitialResourceProvider ().GetThirdPartyEntry (id);

        if (entry.GetType () != VirtualFileSystem::EntryType::FILE)
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to get virtual file system entry for \"", id, "\".");
            return false;
        }

        _context.GetResourceList ().AddThirdParty ({id, entry});
    }

    EMERGENCE_LOG (INFO, "Resource::Cooking: All resource import pass finished successfully.");
    return true;
}
} // namespace Emergence::Resource::Cooking
