#include <Resource/Cooking/ResourceList.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Resource::Cooking
{
const ObjectData::Reflection &ObjectData::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ObjectData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);

        static_assert (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t) ||
                       sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t));

        if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt64 (Memory::UniqueString {"typeNumber"}, offsetof (ObjectData, type));
        }
        else if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt32 (Memory::UniqueString {"typeNumber"}, offsetof (ObjectData, type));
        }

        EMERGENCE_MAPPING_REGISTER_REGULAR (format);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ThirdPartyData::Reflection &ThirdPartyData::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ThirdPartyData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

Container::Optional<ObjectData> ResourceList::AllObjectsCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ObjectData *> (*cursor))
    {
        return *data;
    }

    return {};
}

ResourceList::AllObjectsCursor &ResourceList::AllObjectsCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsCursor::AllObjectsCursor (
    RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

Container::Optional<ObjectData> ResourceList::AllObjectsOfTypeCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ObjectData *> (*cursor))
    {
        return *data;
    }

    return {};
}

ResourceList::AllObjectsOfTypeCursor &ResourceList::AllObjectsOfTypeCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsOfTypeCursor::AllObjectsOfTypeCursor (
    RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

Container::Optional<ThirdPartyData> ResourceList::AllThirdPartyCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ThirdPartyData *> (*cursor))
    {
        return *data;
    }

    return {};
}

ResourceList::AllThirdPartyCursor &ResourceList::AllThirdPartyCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllThirdPartyCursor::AllThirdPartyCursor (
    RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

ResourceList::ResourceList () noexcept
    : objects (ObjectData::Reflect ().mapping),
      allObjects (objects.CreateLinearRepresentation (ObjectData::Reflect ().id)),
      objectsByType (objects.CreatePointRepresentation ({ObjectData::Reflect ().typeNumber})),
      objectsById (objects.CreatePointRepresentation ({ObjectData::Reflect ().id})),

      thirdParty (ThirdPartyData::Reflect ().mapping),
      allThirdParty (thirdParty.CreateLinearRepresentation (ThirdPartyData::Reflect ().id)),
      thirdPartyById (thirdParty.CreatePointRepresentation ({ThirdPartyData::Reflect ().id}))
{
}

void ResourceList::AddObject (const ObjectData &_object) noexcept
{
    auto inserter = objects.AllocateAndInsert ();
    auto *object = static_cast<ObjectData *> (inserter.Allocate ());
    *object = _object;
}

void ResourceList::AddThirdParty (const ThirdPartyData &_thirdParty) noexcept
{
    auto inserter = thirdParty.AllocateAndInsert ();
    auto *thirdPartyInstance = static_cast<ThirdPartyData *> (inserter.Allocate ());
    *thirdPartyInstance = _thirdParty;
}

ResourceList::AllObjectsCursor ResourceList::VisitAllObjects () const noexcept
{
    return {allObjects.ReadAscendingInterval (nullptr, nullptr)};
}

ResourceList::AllObjectsOfTypeCursor ResourceList::VisitAllObjectsOfType (
    const StandardLayout::Mapping &_mapping) const noexcept
{
    return {objectsByType.ReadPoint (&_mapping)};
}

ResourceList::AllThirdPartyCursor ResourceList::VisitAllThirdParty () const noexcept
{
    return {allThirdParty.ReadAscendingInterval (nullptr, nullptr)};
}

Container::Optional<ObjectData> ResourceList::QueryObject (const Memory::UniqueString &_id) const noexcept
{
    auto cursor = objectsById.ReadPoint (&_id);
    if (const auto *object = static_cast<const ObjectData *> (*cursor))
    {
        return *object;
    }

    return {};
}

Container::Optional<ThirdPartyData> ResourceList::QueryThirdParty (const Memory::UniqueString &_id) const noexcept
{
    auto cursor = thirdPartyById.ReadPoint (&_id);
    if (const auto *thirdPartyInstance = static_cast<const ThirdPartyData *> (*cursor))
    {
        return *thirdPartyInstance;
    }

    return {};
}

void ResourceList::Clear () noexcept
{
    objects.Clear ();
    thirdParty.Clear ();
}
} // namespace Emergence::Resource::Cooking
