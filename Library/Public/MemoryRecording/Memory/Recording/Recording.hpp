#pragma once

#include <memory>

#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/Vector.hpp>

#include <Memory/Recording/Event.hpp>

#include <Memory/UniqueString.hpp>
#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory::Recording
{
class RecordedAllocationGroup final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, const RecordedAllocationGroup *);

    private:
        /// RecordedAllocationGroup constructs iterators.
        friend class RecordedAllocationGroup;

        explicit Iterator (const RecordedAllocationGroup *_current) noexcept;

        const RecordedAllocationGroup *current;
    };

    RecordedAllocationGroup (const RecordedAllocationGroup &_other) = delete;

    RecordedAllocationGroup (RecordedAllocationGroup &&_other) = delete;

    ~RecordedAllocationGroup () = default;

    const RecordedAllocationGroup *Parent () const noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    static Iterator EndChildren () noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (RecordedAllocationGroup);

private:
    friend class Recording;

    friend class std::default_delete<RecordedAllocationGroup>;

    static UnorderedPool &GetGroupPool () noexcept;

    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    RecordedAllocationGroup (RecordedAllocationGroup *_parent,
                             Memory::UniqueString _id,
                             std::size_t _reserved,
                             std::size_t _acquired) noexcept;

    Memory::UniqueString id;
    std::size_t reserved = 0u;
    std::size_t acquired = 0u;

    RecordedAllocationGroup *parent = nullptr;
    std::unique_ptr<RecordedAllocationGroup> firstChild;
    std::unique_ptr<RecordedAllocationGroup> nextOnLevel;
};

class Recording final
{
private:
    struct EventNode final
    {
        Event event;
        EventNode *previous = nullptr;
        EventNode *next = nullptr;
    };

    static_assert (std::is_trivially_destructible_v<EventNode>);

public:
    class EventIterator final
    {
    public:
        // NOTE: Circles from end to first and from begin to end.

        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (EventIterator, const Event *);

    private:
        /// Recording constructs iterators.
        friend class Recording;

        explicit EventIterator (const Recording *_recording, const EventNode *_current) noexcept;

        const Recording *recording;
        const EventNode *current;
    };

    Recording () noexcept;

    [[nodiscard]] const RecordedAllocationGroup *Root () const noexcept;

    [[nodiscard]] EventIterator EventBegin () const noexcept;

    [[nodiscard]] EventIterator EventCurrent () const noexcept;

    [[nodiscard]] EventIterator EventEnd () const noexcept;

    bool MoveToPreviousEvent () noexcept;

    bool MoveToNextEvent () noexcept;

    const RecordedAllocationGroup *GetGroupByUID (GroupUID _uid) const noexcept;

private:
    friend class DeserializerBase;

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
