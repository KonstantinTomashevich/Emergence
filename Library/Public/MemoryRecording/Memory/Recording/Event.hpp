#pragma once

#include <chrono>
#include <limits>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Allocation group unique id inside memory usage track.
using GroupUID = std::uint64_t;

/// \brief Special group uid value that is never bound to any group.
constexpr GroupUID MISSING_GROUP_ID = std::numeric_limits<GroupUID>::max ();

/// \brief Types of memory usage recording events.
enum class EventType : std::uint8_t
{
    /// \brief New memory allocation group is declared.
    DECLARE_GROUP = 0u,

    /// \brief Memory is allocated for future usage.
    ALLOCATE,

    /// \brief Memory is being used for something.
    ACQUIRE,

    /// \brief Memory is no longer used.
    RELEASE,

    /// \brief Memory is no longer owned.
    FREE,

    /// \brief Message, used to make event sequence more informative. For example, to highlight fixed frame bounds.
    MARKER
};

/// \brief Contains data of EventType::DECLARE_GROUP events.
struct DeclareGroupEventData
{
    /// \brief UID of the parent group, that must be declared before.
    GroupUID parent;

    /// \brief Human-readable id for new group.
    UniqueString id;

    /// \brief Internal uid id for new group.
    /// \invariant Group uids should be incremental and start from zero.
    /// \details It is advised to use GroupUIDAssigner to assign ids.
    GroupUID uid;

    /// \brief Count of bytes, that are already reserved for future usage.
    std::uint64_t reservedBytes;

    /// \brief Count of bytes, that are already used for something.
    std::uint64_t acquiredBytes;

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::FieldId id;
        StandardLayout::FieldId uid;
        StandardLayout::FieldId reservedBytes;
        StandardLayout::FieldId acquiredBytes;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect ();
};

/// \brief Contains data of EventType::ALLOCATE, EventType::ACQUIRE, EventType::RELEASE and EventType::FREE events.
struct MemoryEventData final
{
    /// \brief UID of the group, associated with this event.
    GroupUID group;

    /// \brief Amount of bytes, used by operation specified by ::type.
    std::uint64_t bytes;

    struct Reflection final
    {
        StandardLayout::FieldId group;
        StandardLayout::FieldId bytes;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect ();
};

/// \brief Contains data of EventType::MARKER events.
struct MarkerEventData final
{
    /// \brief UID of the group, used as a scope for this marker.
    GroupUID scope;

    /// \brief Human-readable id of the marker.
    UniqueString markerId;

    struct Reflection final
    {
        StandardLayout::FieldId scope;
        StandardLayout::FieldId markerId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect ();
};

/// \brief Contains data of a single event in memory usage track.
struct Event
{
    /// \brief Constructor for EventType::DECLARE_GROUP events.
    /// \details Constructors are required, because MSVC reports internal error on field initialization of this struct.
    Event (std::uint64_t _timeNs,
           GroupUID _parent,
           UniqueString _id,
           GroupUID _uid,
           std::uint64_t _reservedBytes,
           std::uint64_t _acquiredBytes) noexcept;

    /// \brief Constructor for EventType::ALLOCATE, EventType::ACQUIRE, EventType::RELEASE and EventType::FREE events.
    /// \details Constructors are required, because MSVC reports internal error on field initialization of this struct.
    Event (EventType _type, std::uint64_t _timeNs, GroupUID _group, std::uint64_t _bytes) noexcept;

    /// \brief Constructor for EventType::MARKER events.
    /// \details Constructors are required, because MSVC reports internal error on field initialization of this struct.
    Event (std::uint64_t _timeNs, GroupUID _scope, UniqueString _markedId) noexcept;

    EventType type;

    /// \brief Time point in nanoseconds after startup, at which event occurred.
    std::uint64_t timeNs;

    union
    {
        /// ::eventType is EventType::DECLARE_GROUP.
        DeclareGroupEventData declareGroup;

        /// ::eventType is EventType::ALLOCATE, EventType::ACQUIRE, EventType::RELEASE or EventType::FREE.
        MemoryEventData memory;

        /// ::eventType is EventType::MARKER.
        MarkerEventData marker;
    };

    struct Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId timeNs;

        StandardLayout::FieldId declareGroup;
        StandardLayout::FieldId memory;
        StandardLayout::FieldId marker;

        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Memory::Recording
