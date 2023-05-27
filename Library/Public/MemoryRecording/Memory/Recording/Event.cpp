#include <Assert/Assert.hpp>

#include <Memory/Recording/Event.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Memory::Recording
{
const DeclareGroupEventData::Reflection &DeclareGroupEventData::Reflect ()
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DeclareGroupEventData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uid);
        EMERGENCE_MAPPING_REGISTER_REGULAR (reservedBytes);
        EMERGENCE_MAPPING_REGISTER_REGULAR (acquiredBytes);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MemoryEventData::Reflection &MemoryEventData::Reflect ()
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MemoryEventData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (group);
        EMERGENCE_MAPPING_REGISTER_REGULAR (bytes);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MarkerEventData::Reflection &MarkerEventData::Reflect ()
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MarkerEventData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (scope);
        EMERGENCE_MAPPING_REGISTER_REGULAR (markerId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

Event::Event (std::uint64_t _timeNs,
              GroupUID _parent,
              UniqueString _id,
              GroupUID _uid,
              std::uint64_t _reservedBytes,
              std::uint64_t _acquiredBytes) noexcept
    : type (EventType::DECLARE_GROUP),
      timeNs (_timeNs),
      declareGroup ({_parent, _id, _uid, _reservedBytes, _acquiredBytes})
{
}

Event::Event (EventType _type, std::uint64_t _timeNs, GroupUID _group, std::uint64_t _bytes) noexcept
    : type (_type),
      timeNs (_timeNs),
      memory ({_group, _bytes})
{
    EMERGENCE_ASSERT (type == EventType::ALLOCATE || type == EventType::ACQUIRE || type == EventType::RELEASE ||
                      type == EventType::FREE);
}

Event::Event (std::uint64_t _timeNs, GroupUID _scope, UniqueString _markedId) noexcept
    : type (EventType::MARKER),
      timeNs (_timeNs),
      marker ({_scope, _markedId})
{
}

const Event::Reflection &Event::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Event);

        EMERGENCE_MAPPING_REGISTER_REGULAR (type);
        EMERGENCE_MAPPING_REGISTER_REGULAR (timeNs);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (
            type, static_cast<std::underlying_type_t<EventType>> (EventType::DECLARE_GROUP));
        EMERGENCE_MAPPING_REGISTER_REGULAR (declareGroup);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        // It is a special case for unions: there is no one-to-one value-fields mapping. Therefore, conditions are
        // written directly. If there would be more cases like this in the future, we need to elevate it into generic
        // registration macro.
        builder.PushVisibilityCondition (reflectionData.type, StandardLayout::ConditionalOperation::GREATER,
                                         static_cast<std::underlying_type_t<EventType>> (EventType::DECLARE_GROUP));
        builder.PushVisibilityCondition (reflectionData.type, StandardLayout::ConditionalOperation::LESS,
                                         static_cast<std::underlying_type_t<EventType>> (EventType::MARKER));
        EMERGENCE_MAPPING_REGISTER_REGULAR (memory);
        builder.PopVisibilityCondition ();
        builder.PopVisibilityCondition ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type,
                                               static_cast<std::underlying_type_t<EventType>> (EventType::MARKER));
        EMERGENCE_MAPPING_REGISTER_REGULAR (marker);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Memory::Recording
