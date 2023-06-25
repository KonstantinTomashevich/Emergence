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

const ObjectData *ResourceList::AllObjectsReadCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ObjectData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllObjectsReadCursor &ResourceList::AllObjectsReadCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsReadCursor::AllObjectsReadCursor (
    RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

ObjectData *ResourceList::AllObjectsEditCursor::operator* () noexcept
{
    if (auto *data = static_cast<ObjectData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllObjectsEditCursor &ResourceList::AllObjectsEditCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsEditCursor &ResourceList::AllObjectsEditCursor::operator~() noexcept
{
    ~cursor;
    return *this;
}

ResourceList::AllObjectsEditCursor::AllObjectsEditCursor (
    RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

const ObjectData *ResourceList::AllObjectsOfTypeReadCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ObjectData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllObjectsOfTypeReadCursor &ResourceList::AllObjectsOfTypeReadCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsOfTypeReadCursor::AllObjectsOfTypeReadCursor (
    RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

ObjectData *ResourceList::AllObjectsOfTypeEditCursor::operator* () noexcept
{
    if (auto *data = static_cast<ObjectData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllObjectsOfTypeEditCursor &ResourceList::AllObjectsOfTypeEditCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllObjectsOfTypeEditCursor &ResourceList::AllObjectsOfTypeEditCursor::operator~() noexcept
{
    ~cursor;
    return *this;
}

ResourceList::AllObjectsOfTypeEditCursor::AllObjectsOfTypeEditCursor (
    RecordCollection::PointRepresentation::EditCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

const ThirdPartyData *ResourceList::AllThirdPartyReadCursor::operator* () const noexcept
{
    if (const auto *data = static_cast<const ThirdPartyData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllThirdPartyReadCursor &ResourceList::AllThirdPartyReadCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllThirdPartyReadCursor::AllThirdPartyReadCursor (
    RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept
    : cursor (std::move (_cursor))
{
}

ThirdPartyData *ResourceList::AllThirdPartyEditCursor::operator* () noexcept
{
    if (auto *data = static_cast<ThirdPartyData *> (*cursor))
    {
        return data;
    }

    return nullptr;
}

ResourceList::AllThirdPartyEditCursor &ResourceList::AllThirdPartyEditCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceList::AllThirdPartyEditCursor &ResourceList::AllThirdPartyEditCursor::operator~() noexcept
{
    ~cursor;
    return *this;
}

ResourceList::AllThirdPartyEditCursor::AllThirdPartyEditCursor (
    RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept
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

Container::Optional<ObjectData> ResourceList::QueryObject (Memory::UniqueString _id) const noexcept
{
    auto cursor = objectsById.ReadPoint (&_id);
    if (const auto *object = static_cast<const ObjectData *> (*cursor))
    {
        return *object;
    }

    return {};
}

Container::Optional<ThirdPartyData> ResourceList::QueryThirdParty (Memory::UniqueString _id) const noexcept
{
    auto cursor = thirdPartyById.ReadPoint (&_id);
    if (const auto *thirdPartyInstance = static_cast<const ThirdPartyData *> (*cursor))
    {
        return *thirdPartyInstance;
    }

    return {};
}

void ResourceList::RemoveObject (Memory::UniqueString _id) noexcept
{
    if (auto cursor = objectsById.EditPoint (&_id); *cursor)
    {
        ~cursor;
    }
}

void ResourceList::RemoveThirdParty (Memory::UniqueString _id) noexcept
{
    if (auto cursor = thirdPartyById.EditPoint (&_id); *cursor)
    {
        ~cursor;
    }
}

ResourceList::AllObjectsReadCursor ResourceList::ReadAllObjects () const noexcept
{
    return {allObjects.ReadAscendingInterval (nullptr, nullptr)};
}

ResourceList::AllObjectsEditCursor ResourceList::EditAllObjects () noexcept
{
    return {allObjects.EditAscendingInterval (nullptr, nullptr)};
}

ResourceList::AllObjectsOfTypeReadCursor ResourceList::ReadAllObjectsOfType (
    const StandardLayout::Mapping &_mapping) const noexcept
{
    return {objectsByType.ReadPoint (&_mapping)};
}

ResourceList::AllObjectsOfTypeEditCursor ResourceList::EditAllObjectsOfType (
    const StandardLayout::Mapping &_mapping) noexcept
{
    return {objectsByType.EditPoint (&_mapping)};
}

ResourceList::AllThirdPartyReadCursor ResourceList::ReadAllThirdParty () const noexcept
{
    return {allThirdParty.ReadAscendingInterval (nullptr, nullptr)};
}

ResourceList::AllThirdPartyEditCursor ResourceList::EditAllThirdParty () noexcept
{
    return {allThirdParty.EditAscendingInterval (nullptr, nullptr)};
}

void ResourceList::Clear () noexcept
{
    objects.Clear ();
    thirdParty.Clear ();
}
} // namespace Emergence::Resource::Cooking
