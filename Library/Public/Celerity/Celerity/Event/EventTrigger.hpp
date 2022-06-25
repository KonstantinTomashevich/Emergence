#pragma once

#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Celerity/Pipeline.hpp>

#include <Memory/Heap.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Warehouse/InsertShortTermQuery.hpp>

namespace Emergence::Celerity
{
/// \brief Describes how event should be processed in context of world pipelines.
enum class EventRoute
{
    /// \brief Can be created and accessed only during fixed update.
    /// \details Accessible both in creation frame and the next frame, cleared right before creation of new ones.
    /// \invariant All tasks, that create or access event type, should be grouped like that:
    ///            (next frame accessors) -> (event creators) -> (current frame accessors)
    ///            in order to correctly process events without leaks or partial retrievals.
    FIXED = 0u,

    /// \brief The same as ::FIXED, but can be created only during normal update.
    NORMAL,

    /// \brief Can be created only during fixed update and can be accessed only during normal update.
    /// \details Cleared in the end of normal update.
    FROM_FIXED_TO_NORMAL,

    /// \brief Can be created and accessed inside any custom pipeline. Cleared in the end of custom pipelines.
    CUSTOM,

    /// \brief Can be created during any custom pipeline execution and can be accessed only during fixed update.
    /// \details Cleared in the end of fixed update.
    FROM_CUSTOM_TO_FIXED,

    /// \brief Can be created during any custom pipeline execution and can be accessed only during normal update.
    /// \details Cleared in the end of normal update.
    FROM_CUSTOM_TO_NORMAL,

    COUNT,
};

/// \return Pipeline type, that is allowed to produce events.
PipelineType GetEventProducingPipeline (EventRoute _route) noexcept;

/// \return Pipeline type, that is allowed to consume events.
PipelineType GetEventConsumingPipeline (EventRoute _route) noexcept;

/// \brief Pair of record-event fields, used for automatic copying when event is fired.
/// \invariant Both fields have the same archetype and size.
/// \invariant FieldArchetype::BIT is not supported.
/// \invariant Field must be trivially copyable.
struct CopyOutField final
{
    StandardLayout::FieldId recordField;
    StandardLayout::FieldId eventField;
};

/// \brief Result of CopyOutFieldProcessing, that is more performance-friendly.
/// \details Commands to copy one or more fields, that are positioned next to each other.
struct CopyOutBlock final
{
    std::size_t sourceOffset = 0u;
    std::size_t targetOffset = 0u;
    std::size_t length = 0u;
};

/// \brief Maximum amount of CopyOutBlock's per event.
/// \details For performance reasons, CopyOutBlock's are stored in inplace vectors, therefore maximum amount of
///          these blocks per event is fixed. This value can be increased, but it will increase memory usage as well.
/// TODO: Move such constants to constant sub library, like Pegasus?
constexpr std::size_t MAX_COPY_OUT_BLOCKS_PER_EVENT = 4u;

/// \brief Contains common data for all event triggers.
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

/// \brief Trigger for trivial automated events like OnAdd/OnRemove.
class TrivialEventTrigger final : public EventTriggerBase
{
public:
    TrivialEventTrigger (StandardLayout::Mapping _trackedType,
                         Warehouse::InsertShortTermQuery _inserter,
                         EventRoute _route,
                         const Container::Vector<CopyOutField> &_copyOuts) noexcept;

    /// \brief Trigger event for given record.
    void Trigger (const void *_record) noexcept;

private:
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOuts;
};

/// \brief Trivial OnAdd/OnRemove events for the same tracked type should be stored in rows to make event firing easier.
/// \details Because of event routing, there could be multiple OnAdd/OnRemove events per tracked type,
///          but not more than routes count.
using TrivialEventTriggerRow =
    Container::InplaceVector<TrivialEventTrigger, static_cast<std::size_t> (EventRoute::COUNT)>;

constexpr std::size_t MAX_TRACKED_ZONES_PER_EVENT = 4u;

/// \brief Trigger for OnChange automated events.
class OnChangeEventTrigger final : public EventTriggerBase
{
public:
    OnChangeEventTrigger (StandardLayout::Mapping _trackedType,
                          Warehouse::InsertShortTermQuery _inserter,
                          EventRoute _route,
                          const Container::Vector<StandardLayout::FieldId> &_trackedFields,
                          const Container::Vector<CopyOutField> &_copyOutOfInitial,
                          const Container::Vector<CopyOutField> &_copyOutOfChanged) noexcept;

    /// \brief Trigger event for given changed record with its tracking buffer (see ChangeTracker).
    void Trigger (const void *_changedRecord, const void *_trackingBuffer) noexcept;

private:
    friend class ChangeTracker;

    struct TrackedZone final
    {
        std::size_t offset = 0u;
        std::size_t length = 0u;
    };

    void BakeTrackedFields (const StandardLayout::Mapping &_recordType,
                            const Container::Vector<StandardLayout::FieldId> &_fields) noexcept;

    Container::InplaceVector<TrackedZone, MAX_TRACKED_ZONES_PER_EVENT> trackedZones;
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOutOfInitial;
    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> copyOutOfChanged;
};

/// \brief Maximum count of OnChange events per type.
/// \details For performance reasons, ChangeTracker stores info about OnChange events in inplace vector,
///          therefore maximum count of events is fixed. This value can be increased, but it will increase
///          memory usage as well.
constexpr std::size_t MAX_ON_CHANGE_EVENTS_PER_TYPE = 4u;

class ChangeTracker final
{
public:
    using EventVector = Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE>;

    ChangeTracker (const EventVector &_events) noexcept;

    ChangeTracker (const ChangeTracker &_other) = delete;

    ChangeTracker (ChangeTracker &&_other) noexcept;

    ~ChangeTracker () = default;

    /// \brief Inform tracker that user started editing given record.
    void BeginEdition (const void *_record) noexcept;

    /// \brief Inform tracker that user finished editing given record.
    /// \details OnChange events are triggered there if any relevant changes are detected.
    void EndEdition (const void *_record) noexcept;

    [[nodiscard]] StandardLayout::Mapping GetTrackedType () const noexcept;

    [[nodiscard]] EventVector GetEventTriggers () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ChangeTracker);

private:
    /// \details Fixed for performance. This value can be increased, but it will increase memory usage as well.
    static constexpr std::size_t MAX_TRACKED_ZONES = 4u;

    /// \details Fixed for performance. This value can be increased, but it will increase memory usage as well.
    static constexpr std::size_t MAX_TRACKING_BUFFER_SIZE = 256u;

    struct TrackedZone final
    {
        std::size_t sourceOffset = 0u;
        std::size_t length = 0u;
        std::uint8_t *buffer = nullptr;
    };

    struct EventBinding final
    {
        OnChangeEventTrigger *event = nullptr;
        uint8_t zoneMask = 0u;

        static_assert (sizeof (zoneMask) * 8u >= MAX_TRACKED_ZONES);
    };

    void BakeTrackedZones (const EventVector &_events) noexcept;

    void BakeBindings (const EventVector &_events) noexcept;

    StandardLayout::Mapping trackedType;
    Container::InplaceVector<TrackedZone, MAX_TRACKED_ZONES> trackedZones;
    Container::InplaceVector<EventBinding, MAX_ON_CHANGE_EVENTS_PER_TYPE> bindings;
    std::array<uint8_t, MAX_TRACKING_BUFFER_SIZE> buffer;
};
} // namespace Emergence::Celerity

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Celerity::CopyOutField)
