#include <Assert/Assert.hpp>

#include <Resource/Object/Object.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Resource::Object
{
const ObjectComponent::Reflection &ObjectComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ObjectComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (component);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Object::Reflection &Object::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN_WITH_CUSTOM_NAME (Object, "ResourceObject");
        EMERGENCE_MAPPING_REGISTER_REGULAR (parent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (changelist);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

static bool DoPatchesReferenceSamePart (const TypeManifest &_typeManifest,
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

    EMERGENCE_ASSERT (_first.GetTypeMapping ().GetField (typeInfo->uniqueId).IsHandleValid ());
    const StandardLayout::FieldArchetype archetype =
        _first.GetTypeMapping ().GetField (typeInfo->uniqueId).GetArchetype ();
    EMERGENCE_ASSERT (archetype == StandardLayout::FieldArchetype::UINT ||
                      archetype == StandardLayout::FieldArchetype::UNIQUE_STRING);
    EMERGENCE_ASSERT (_first.GetTypeMapping ().GetField (typeInfo->uniqueId).GetSize () == sizeof (UniqueId));

    UniqueId firstId = 0u;
    UniqueId secondId = 0u;

    auto findId = [typeInfo, archetype] (const StandardLayout::Patch &_patch, UniqueId &_output)
    {
        for (const StandardLayout::Patch::ChangeInfo &changeInfo : _patch)
        {
            if (changeInfo.field == typeInfo->uniqueId)
            {
                if (archetype == StandardLayout::FieldArchetype::UINT)
                {
                    _output = *static_cast<const UniqueId *> (changeInfo.newValue);
                }
                else
                {
                    static_assert (sizeof (UniqueId) == sizeof (std::uintptr_t));
                    _output =
                        reinterpret_cast<UniqueId> (**static_cast<const Memory::UniqueString *> (changeInfo.newValue));
                }

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

void ApplyInheritance (const TypeManifest &_typeManifest, const Object &_parent, Object &_child) noexcept
{
    // Algorithm below is not very effective as it uses plain cycles.
    // However, this operation is only needed during loading and objects are not very big, so it is okay.
    Container::Vector<ObjectComponent> resultChangelist {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Object"}}};

    for (const ObjectComponent &parentComponent : _parent.changelist)
    {
        bool overrideFound = false;
        for (const ObjectComponent &childComponent : _child.changelist)
        {
            if (DoPatchesReferenceSamePart (_typeManifest, parentComponent.component, childComponent.component))
            {
                overrideFound = true;
                resultChangelist.emplace_back (ObjectComponent {parentComponent.component + childComponent.component});
                break;
            }
        }

        if (!overrideFound)
        {
            resultChangelist.emplace_back (parentComponent);
        }
    }

    for (const ObjectComponent &childComponent : _child.changelist)
    {
        bool alreadyUsedAsOverride = false;
        for (const ObjectComponent &parentComponent : _parent.changelist)
        {
            if (DoPatchesReferenceSamePart (_typeManifest, parentComponent.component, childComponent.component))
            {
                alreadyUsedAsOverride = true;
                break;
            }
        }

        if (!alreadyUsedAsOverride)
        {
            resultChangelist.emplace_back (childComponent);
        }
    }

    _child.changelist = resultChangelist;
}

void ExtractChildChangelist (const TypeManifest &_typeManifest,
                             const Object &_parent,
                             const Object &_child,
                             Container::Vector<ObjectComponent> &_output) noexcept
{
    // Algorithm below is not very effective as it uses plain cycles.
    // However, this operation is only needed during saving and objects are not very big, so it is okay.

    for (const ObjectComponent &childComponent : _child.changelist)
    {
        bool foundInParent = false;
        for (const ObjectComponent &parentComponent : _parent.changelist)
        {
            if (childComponent.component.IsHandleEqual (parentComponent.component))
            {
                foundInParent = true;
                break;
            }

            if (!DoPatchesReferenceSamePart (_typeManifest, childComponent.component, parentComponent.component))
            {
                continue;
            }

            foundInParent = true;
            _output.emplace_back (ObjectComponent {childComponent.component - parentComponent.component});
        }

        if (!foundInParent)
        {
            _output.emplace_back (childComponent);
        }
    }
}
} // namespace Emergence::Resource::Object
