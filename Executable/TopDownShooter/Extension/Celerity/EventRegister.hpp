#pragma once

#include <utility>

#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Memory/Heap.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Warehouse/InsertShortTermQuery.hpp>

namespace Emergence::Celerity
{
// TODO: This routes should be used to generate clearing tasks and validate event usage.
enum class EventRoute
{
    /// \brief Can be created and accessed only during fixed update.
    /// \details Accessible both in creation frame and the next frame, cleared right before creation of new ones.
    /// \invariant All tasks, that create or access event type, should be grouped like that:
    ///            (next frame accessors) -> (event creators) -> (current frame accessors)
    ///            in order to be correctly processed with event leaking or partial retrieval.
    FIXED = 0u,

    /// \brief The same as ::FIXED, but can be created only during normal update.
    NORMAL,

    /// \brief Can be created only during fixed update and can be accessed only during normal update.
    /// \details Cleared in the end of normal update.
    FROM_FIXED_TO_NORMAL,

    /// \brief Can be created and accessed inside any custom pipeline. Cleared in the end of custom pipelines.
    CUSTOM,
};

struct CopyOutField final
{
    StandardLayout::FieldId recordField;
    StandardLayout::FieldId eventField;
};

struct CopyOutBlock final
{
    std::size_t sourceOffset = 0u;
    std::size_t targetOffset = 0u;
    std::size_t length = 0u;
};

constexpr std::size_t MAX_COPY_OUT_BLOCKS_PER_EVENT = 4u;

class EventTriggerBase
{
public:
    [[nodiscard]] StandardLayout::Mapping GetTrackedType () const noexcept;

    [[nodiscard]] StandardLayout::Mapping GetEventType () const noexcept;

    [[nodiscard]] EventRoute GetRoute () const noexcept;

protected:
    EventTriggerBase (StandardLayout::Mapping _trackedType,
                      Warehouse::InsertShortTermQuery _inserter,
                      EventRoute _route) noexcept;

    StandardLayout::Mapping trackedType;
    Warehouse::InsertShortTermQuery inserter;
    EventRoute route;
};

class TrivialEventTrigger final : public EventTriggerBase
{
public:
    TrivialEventTrigger (StandardLayout::Mapping _trackedType,
                         Warehouse::InsertShortTermQuery _inserter,
                         EventRoute _route,
                         const Container::Vector<CopyOutField>& _copyOuts) noexcept;

    void Trigger (const void *_record) noexcept;

private:
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOuts;
};

struct TrackedZone final
{
    std::size_t offset = 0u;
    std::size_t length = 0u;
};

constexpr std::size_t MAX_TRACKED_ZONES_PER_EVENT = 4u;

class OnChangeEventTrigger final : public EventTriggerBase
{
public:
    OnChangeEventTrigger (StandardLayout::Mapping _trackedType,
                          Warehouse::InsertShortTermQuery _inserter,
                          EventRoute _route,
                          const Container::Vector<StandardLayout::FieldId>& _trackedFields,
                          const Container::Vector<CopyOutField>& _copyOutOfInitial,
                          const Container::Vector<CopyOutField>& _copyOutOfChanged) noexcept;

    void Trigger (const void *_changedRecord, const void *_initialRecord) noexcept;

private:
    friend class ChangeTracker;

    Container::InplaceVector<TrackedZone, MAX_TRACKED_ZONES_PER_EVENT> trackedZones;
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOutOfInitial;
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOutOfChanged;
};

constexpr std::size_t MAX_ON_CHANGE_EVENTS_PER_TYPE = 4u;

class ChangeTracker final
{
public:
    ChangeTracker (
        const Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE> &_events) noexcept;

    ChangeTracker (const ChangeTracker &_other) = delete;

    ChangeTracker (ChangeTracker &&_other) noexcept;

    ~ChangeTracker () noexcept;

    void StartEdition (const void *_record) noexcept;

    void EndEdition (const void *_record) noexcept;

    [[nodiscard]] StandardLayout::Mapping GetTrackedType () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ChangeTracker);

private:
    static constexpr std::size_t MAX_TRACKED_ZONES = 4u;

    struct EventBinding final
    {
        OnChangeEventTrigger *event = nullptr;
        uint8_t zoneMask = 0u;

        static_assert (sizeof (zoneMask) * 8u >= MAX_TRACKED_ZONES);
    };

    StandardLayout::Mapping trackedType;

    // TODO: Any better way to allocate buffers?
    Memory::Heap bufferHeap;

    void *buffer;

    Container::InplaceVector<TrackedZone, MAX_TRACKED_ZONES> trackedZones;
    Container::InplaceVector<EventBinding, MAX_ON_CHANGE_EVENTS_PER_TYPE> bindings;
};

// NOTE: Currently events are planned only for long term objects, because it looks strange to support them for short
//       term ones. Also, support for short term is difficult, because we would need to pass Celerity version of
//       InsertShortTermQuery instead of Warehouse one, which would add one more step to backing.
} // namespace Emergence::Celerity

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Celerity::CopyOutField)
