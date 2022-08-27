#include <Asset/Object/Object.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Asset::Object
{
const Declaration::Reflection &Declaration::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Declaration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parent);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

static bool ArePatchesReferenceSamePart (const TypeManifest &_typeManifest,
                                         const StandardLayout::Patch &_first,
                                         const StandardLayout::Patch &_second)
{
    if (_first.GetTypeMapping () != _second.GetTypeMapping ())
    {
        return false;
    }

    const TypeInfo *typeInfo = _typeManifest.Get (_first.GetTypeMapping ());
    if (!typeInfo)
    {
        return false;
    }

    assert (_first.GetTypeMapping ().GetField (typeInfo->uniqueId).IsHandleValid ());
    assert (_first.GetTypeMapping ().GetField (typeInfo->uniqueId).GetArchetype () ==
            StandardLayout::FieldArchetype::UINT);
    assert (_first.GetTypeMapping ().GetField (typeInfo->uniqueId).GetSize () == sizeof (UniqueId));

    UniqueId firstId = 0u;
    UniqueId secondId = 0u;

    auto findId = [typeInfo] (const StandardLayout::Patch &_patch, UniqueId &_output)
    {
        for (const StandardLayout::Patch::ChangeInfo &changeInfo : _patch)
        {
            if (changeInfo.field == typeInfo->uniqueId)
            {
                _output = *static_cast<const UniqueId *> (changeInfo.newValue);
                return true;
            }
        }

        return false;
    };

    const bool firstIdFound = findId (_first, firstId);
    const bool secondIdFound = findId (_second, secondId);

    // If both IDs are not found, then both ids have default value and therefore are equal.
    if (firstIdFound != secondIdFound)
    {
        return false;
    }

    return firstId == secondId;
}

Body ApplyInheritance (const TypeManifest &_typeManifest,
                       const Body &_parent,
                       const Container::Vector<StandardLayout::Patch> &_childChangelist) noexcept
{
    // Algorithm below is not very effective as it uses plain cycles.
    // However, this operation is only needed during loading and objects are not very big, so it is okay.
    Body result;

    for (const StandardLayout::Patch &parentPatch : _parent.fullChangelist)
    {
        bool overrideFound = false;
        for (const StandardLayout::Patch &childPatch : _childChangelist)
        {
            if (ArePatchesReferenceSamePart (_typeManifest, parentPatch, childPatch))
            {
                overrideFound = true;
                result.fullChangelist.emplace_back (parentPatch + childPatch);
                break;
            }
        }

        if (!overrideFound)
        {
            result.fullChangelist.emplace_back (parentPatch);
        }
    }

    for (const StandardLayout::Patch &childPatch : _childChangelist)
    {
        bool alreadyUsedAsOverride = false;
        for (const StandardLayout::Patch &parentPatch : _parent.fullChangelist)
        {
            if (ArePatchesReferenceSamePart (_typeManifest, parentPatch, childPatch))
            {
                alreadyUsedAsOverride = true;
                break;
            }
        }

        if (!alreadyUsedAsOverride)
        {
            result.fullChangelist.emplace_back (childPatch);
        }
    }

    return result;
}

void ExtractChildChangelist (const TypeManifest &_typeManifest,
                             const Body &_parent,
                             const Body &_child,
                             Container::Vector<StandardLayout::Patch> &_output) noexcept
{
    // Algorithm below is not very effective as it uses plain cycles.
    // However, this operation is only needed during saving and objects are not very big, so it is okay.

    for (const StandardLayout::Patch &childPatch : _child.fullChangelist)
    {
        bool foundInParent = false;
        for (const StandardLayout::Patch &parentPatch : _parent.fullChangelist)
        {
            if (childPatch.IsHandleEqual (parentPatch))
            {
                foundInParent = true;
                break;
            }

            if (!ArePatchesReferenceSamePart (_typeManifest, childPatch, parentPatch))
            {
                continue;
            }

            foundInParent = true;
            _output.emplace_back (childPatch - parentPatch);
        }

        if (!foundInParent)
        {
            _output.emplace_back (childPatch);
        }
    }
}
} // namespace Emergence::Asset::Object
