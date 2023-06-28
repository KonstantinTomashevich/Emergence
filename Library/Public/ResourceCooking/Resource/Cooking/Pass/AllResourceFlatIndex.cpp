#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Pass/AllResourceFlatIndex.hpp>

#include <Resource/Provider/IndexFile.hpp>

namespace Emergence::Resource::Cooking
{
bool AllResourceFlatIndexPass (Context &_context) noexcept
{
    EMERGENCE_LOG (INFO, "Resource::Cooking: All resource flat index pass started.");

    const VirtualFileSystem::Entry passRealDirectory =
        _context.GetPassIntermediateRealDirectory ("AllResourceFlatIndex");

    VirtualFileSystem::Entry outputEntry {passRealDirectory, Provider::IndexFile::INDEX_FILE_NAME};
    if (!outputEntry)
    {
        outputEntry =
            _context.GetVirtualFileSystem ().CreateFile (passRealDirectory, Provider::IndexFile::INDEX_FILE_NAME);

        if (!outputEntry)
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to create file for index backing");
            return false;
        }
    }

    const VirtualFileSystem::Entry passVirtualDirectory =
        _context.GetPassIntermediateVirtualDirectory ("AllResourceFlatIndex");

    for (auto cursor = _context.GetResourceList ().ReadAllObjects (); const ObjectData *object = *cursor; ++cursor)
    {
        if (!_context.GetVirtualFileSystem ().CreateWeakFileLink (object->entry, passVirtualDirectory,
                                                                  object->entry.GetFullName ()))
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to create weak link to \"", object->entry.GetFullName (),
                           "\".");
            return false;
        }
    }

    for (auto cursor = _context.GetResourceList ().ReadAllThirdParty (); const ThirdPartyData *thirdParty = *cursor;
         ++cursor)
    {
        if (!_context.GetVirtualFileSystem ().CreateWeakFileLink (thirdParty->entry, passVirtualDirectory,
                                                                  thirdParty->entry.GetFullName ()))
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to create weak link to \"",
                           thirdParty->entry.GetFullName (), "\".");
            return false;
        }
    }

    Provider::ResourceProvider bakingProvider {&_context.GetVirtualFileSystem (),
                                               _context.GetInitialResourceProvider ().GetObjectTypesRegistry (),
                                               _context.GetInitialResourceProvider ().GetPatchableTypesRegistry ()};
    Memory::UniqueString virtualSource {passVirtualDirectory.GetFullPath ().c_str ()};

    if (Provider::SourceOperationResponse response = bakingProvider.AddSource (virtualSource);
        response != Provider::SourceOperationResponse::SUCCESSFUL)
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to add source \"", virtualSource,
                       "\" for index baking, response code: ", static_cast<std::uint16_t> (response), ".");
        return false;
    }

    if (Provider::SourceOperationResponse response = bakingProvider.SaveSourceIndex (virtualSource, outputEntry);
        response != Provider::SourceOperationResponse::SUCCESSFUL)
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed bake index for virtual source \"", virtualSource,
                       "\", response code: ", static_cast<std::uint16_t> (response), ".");
        return false;
    }

    _context.GetResourceList ().AddObject ({
        Memory::UniqueString {Provider::IndexFile::INDEX_FILE_NAME},
        Provider::IndexFile::Reflect ().mapping,
        outputEntry,
        Provider::ObjectFormat::BINARY,
    });

    EMERGENCE_LOG (INFO, "Resource::Cooking: All resource flat index pass finished successfully.");
    return true;
}
} // namespace Emergence::Resource::Cooking
