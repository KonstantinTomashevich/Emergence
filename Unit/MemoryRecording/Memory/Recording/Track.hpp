#pragma once

#include <MemoryRecordingApi.hpp>

#include <memory>

#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/Vector.hpp>

#include <Memory/Recording/Event.hpp>

#include <Memory/UniqueString.hpp>
#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Stores state of AllocationGroup at current moment of owner Track.
class MemoryRecordingApi RecordedAllocationGroup final
{
public:
    /// \brief Provides iteration over RecordedAllocationGroup children.
    class MemoryRecordingApi Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, const RecordedAllocationGroup *);

    private:
        /// RecordedAllocationGroup constructs iterators.
        friend class RecordedAllocationGroup;

        explicit Iterator (const RecordedAllocationGroup *_current) noexcept;

        const RecordedAllocationGroup *current;
    };

    /// Groups are managed by Track, therefore copying and assigning is forbidden.
    RecordedAllocationGroup (const RecordedAllocationGroup &_other) = delete;

    /// Groups are managed by Track, therefore copying and assigning is forbidden.
    RecordedAllocationGroup (RecordedAllocationGroup &&_other) = delete;

    ~RecordedAllocationGroup () = default;

    /// \return Allocation group that contains this group as subgroup, or `nullptr` if this group is root.
    [[nodiscard]] const RecordedAllocationGroup *Parent () const noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    [[nodiscard]] Iterator EndChildren () const noexcept;

    /// \return Id of this group, unique among children of its ::Parent.
    [[nodiscard]] UniqueString GetId () const noexcept;

    /// \return Amount of bytes, that are used by any object.
    [[nodiscard]] std::size_t GetAcquired () const noexcept;

    /// \return Amount of bytes, that are not used by any object, but reserved for usage in future.
    [[nodiscard]] std::size_t GetReserved () const noexcept;

    /// \return Total amount of bytes, that are owned by allocation group.
    [[nodiscard]] std::size_t GetTotal () const noexcept;

    /// Groups are managed by Track, therefore copying and assigning is forbidden.
    EMERGENCE_DELETE_ASSIGNMENT (RecordedAllocationGroup);

private:
    friend class Track;

    friend struct std::default_delete<RecordedAllocationGroup>;

    static UnorderedPool &GetGroupPool () noexcept;

    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    RecordedAllocationGroup (RecordedAllocationGroup *_parent,
                             Memory::UniqueString _id,
                             std::size_t _reserved,
                             std::size_t _acquired) noexcept;

    void Allocate (std::uint64_t _bytes) noexcept;

    bool Acquire (std::uint64_t _bytes) noexcept;

    bool Release (std::uint64_t _bytes) noexcept;

    bool Free (std::uint64_t _bytes) noexcept;

    Memory::UniqueString id;
    std::size_t reserved = 0u;
    std::size_t acquired = 0u;

    RecordedAllocationGroup *parent = nullptr;
    std::unique_ptr<RecordedAllocationGroup> firstChild;
    std::unique_ptr<RecordedAllocationGroup> nextOnLevel;
};

/// \brief Stores memory usage history as sequence of events and provides allocation
///        group state at any moment of time, selected through current event pointer.
/// \details Events could be reported through any ReporterBase derived class, like RuntimeReporter
///          or StreamDeserializer. Event addition never invalidates iterators.
class MemoryRecordingApi Track final
{
private:
    /// \brief Internal class for storing position in sequence along with event data.
    struct EventNode final
    {
        Event event;
        EventNode *previous = nullptr;
        EventNode *next = nullptr;
    };

    static_assert (std::is_trivially_destructible_v<EventNode>);

public:
    /// \brief Provides iteration over events, stored in Track.
    /// \details For convenience, implements circling behaviour:
    ///          - Incrementing `end` will return `begin`.
    ///          - Decrementing `begin` will return `end`.
    class EventIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (EventIterator, const Event *);

    private:
        /// Track constructs iterators.
        friend class Track;

        explicit EventIterator (const Track *_track, const EventNode *_current) noexcept;

        const Track *track;
        const EventNode *current;
    };

    /// \brief Constructs empty track.
    Track () noexcept;

    Track (const Track &_other) = delete;

    /// \warning Invalidates all iterators and reporters, associated with given track instance.
    Track (Track &&_other) = default;

    ~Track () = default;

    /// \return State of the root allocation group at current moment.
    [[nodiscard]] const RecordedAllocationGroup *Root () const noexcept;

    /// \return Iterator, that points to first reported event.
    /// \details If there are no reported events, equal to ::EventEnd.
    [[nodiscard]] EventIterator EventBegin () const noexcept;

    /// \return Iterator, that points to current (last applied) event.
    /// \details Equal to ::EventEnd if track is in the initial state (no events applied).
    [[nodiscard]] EventIterator EventCurrent () const noexcept;

    /// \return Iterator, that points to the end of events sequence.
    [[nodiscard]] EventIterator EventEnd () const noexcept;

    /// \brief Undo current event and move to previous.
    /// \return True if current event was successfully undone. Also returns `false` if track is at initial state.
    bool MoveToPreviousEvent () noexcept;

    /// \brief Apply next event and make it current.
    /// \return True if next event was successfully applied. Also returns `false` if all reported events are applied.
    bool MoveToNextEvent () noexcept;

    /// \return Current state of a group, associated with given uid, or `nullptr` if there is no group with given uid.
    [[nodiscard]] const RecordedAllocationGroup *GetGroupByUID (GroupUID _uid) const noexcept;

    /// \brief Reset track into initial state by removing all events and groups.
    void Clear () noexcept;

    /// Assigning tracks seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Track);

private:
    friend class ReporterBase;

    void ReportEvent (const Event &_event) noexcept;

    [[nodiscard]] RecordedAllocationGroup *RequireGroup (GroupUID _uid) const noexcept;

    bool ApplyDeclareGroupEvent (const Event &_event) noexcept;

    bool ApplyAllocateEvent (const Event &_event) noexcept;

    bool ApplyAcquireEvent (const Event &_event) noexcept;

    bool ApplyReleaseEvent (const Event &_event) noexcept;

    bool ApplyFreeEvent (const Event &_event) noexcept;

    bool UndoDeclareGroupEvent (const Event &_event) noexcept;

    bool UndoAllocateEvent (const Event &_event) noexcept;

    bool UndoAcquireEvent (const Event &_event) noexcept;

    bool UndoReleaseEvent (const Event &_event) noexcept;

    bool UndoFreeEvent (const Event &_event) noexcept;

    std::unique_ptr<RecordedAllocationGroup> root;
    Container::Vector<RecordedAllocationGroup *> idToGroup;

    UnorderedPool events;
    EventNode *first = nullptr;
    EventNode *last = nullptr;
    EventNode *current = nullptr;
};
} // namespace Emergence::Memory::Recording
