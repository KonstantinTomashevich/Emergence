#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Result/FlatPackage.hpp>

#include <VirtualFileSystem/PackageBuilder.hpp>

namespace Emergence::Resource::Cooking
{
bool ProduceFlatPackage (Context &_context, const std::string_view &_packageFileName) noexcept
{
    EMERGENCE_LOG (INFO, "Resource::Cooking: Producing flat package \"", _packageFileName, "\".");
    VirtualFileSystem::Entry outputEntry {_context.GetFinalResultDirectory (), _packageFileName};

    if (!outputEntry)
    {
        outputEntry =
            _context.GetVirtualFileSystem ().CreateFile (_context.GetFinalResultDirectory (), _packageFileName);

        if (!outputEntry)
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to create package output file \"", _packageFileName,
                           "\".");
            return false;
        }
    }

    VirtualFileSystem::PackageBuilder builder;
    if (!builder.Begin (_context.GetVirtualFileSystem (), outputEntry))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to begin package building with \"", outputEntry.GetFullPath (),
                       "\" output.");
        return false;
    }

    for (auto cursor = _context.GetResourceList ().ReadAllObjects (); const ObjectData *object = *cursor; ++cursor)
    {
        if (!builder.Add (object->entry, object->entry.GetFullName ()))
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to add file \"", object->entry.GetFullPath (),
                           "\" to package.");
            return false;
        }
    }

    for (auto cursor = _context.GetResourceList ().ReadAllThirdParty (); const ThirdPartyData *thirdParty = *cursor;
         ++cursor)
    {
        if (!builder.Add (thirdParty->entry, thirdParty->entry.GetFullName ()))
        {
            EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to add file \"", thirdParty->entry.GetFullPath (),
                           "\" to package.");
            return false;
        }
    }

    if (!builder.End ())
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to end package building with \"", outputEntry.GetFullPath (),
                       "\" output.");
        return false;
    }

    EMERGENCE_LOG (INFO, "Resource::Cooking: Successfully produced flat package \"", _packageFileName, "\".");
    return true;
}
} // namespace Emergence::Resource::Cooking
