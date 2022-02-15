#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <cassert>

#include <Celerity/Event/EventTrigger.hpp>

namespace Emergence
{
namespace Celerity
{
using namespace Memory::Literals;

static void ApplyCopyOut (const CopyOutBlock &_block, const void *_source, void *_target)
{
    assert (_source);
    assert (_target);

    memcpy (static_cast<uint8_t *> (_target) + _block.targetOffset,
            static_cast<const uint8_t *> (_source) + _block.sourceOffset, _block.length);
}

static Memory::Profiler::AllocationGroup GetEventRegistrationAlgorithmsGroup ()
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (),
                                                    "EventRegistrationAlgorithms"_us};
    return group;
}

// TODO: Is there any way to prevent block separation by padding?
static Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> BakeCopyOuts (
    const StandardLayout::Mapping &_recordType,
    const StandardLayout::Mapping &_eventType,
    const Container::Vector<CopyOutField> &_copyOuts)
{
    if (_copyOuts.empty ())
    {
        return {};
    }

    Container::Vector<CopyOutBlock> converted {GetEventRegistrationAlgorithmsGroup ()};
    for (const CopyOutField &copyOut : _copyOuts)
    {
        StandardLayout::Field source = _recordType.GetField (copyOut.recordField);
        StandardLayout::Field target = _eventType.GetField (copyOut.eventField);

        assert (source);
        assert (target);
        assert (source.GetArchetype () == target.GetArchetype ());
        assert (source.GetSize () == target.GetSize ());

        // TODO: Is there any way to add bit support without downgrading overall performance?
        assert (source.GetArchetype () != StandardLayout::FieldArchetype::BIT);

        converted.emplace_back (CopyOutBlock {source.GetOffset (), target.GetOffset (), source.GetSize ()});
    }

    // In most cases input is already sorted (because it's more convenient to specify fields in order of their
    // declaration), therefore sort should be almost free here.
    std::sort (converted.begin (), converted.end (),
               [] (const CopyOutBlock &_first, const CopyOutBlock &_second)
               {
                   return _first.sourceOffset < _second.sourceOffset;
               });

    Container::InplaceVector<CopyOutBlock, MAX_COPY_OUT_BLOCKS_PER_EVENT> result;
    result.EmplaceBack (converted.front ());

    if (converted.size () == 1u)
    {
        return result;
    }

    for (auto iterator = std::next (converted.begin ()); iterator != converted.end (); ++iterator)
    {
        if (result.Back ().sourceOffset + result.Back ().length == iterator->sourceOffset &&
            result.Back ().targetOffset + result.Back ().length == iterator->targetOffset)
        {
            result.Back ().length += iterator->length;
        }
        else
        {
            result.EmplaceBack (*iterator);
        }
    }

    return result;
}

static Container::InplaceVector<TrackedZone, MAX_COPY_OUT_BLOCKS_PER_EVENT> BakeTrackedFields (
    const StandardLayout::Mapping &_recordType, const Container::Vector<StandardLayout::FieldId> &_fields)
{
    if (_fields.empty ())
    {
        return {};
    }

    Container::Vector<TrackedZone> converted {GetEventRegistrationAlgorithmsGroup ()};
    for (const StandardLayout::FieldId fieldId : _fields)
    {
        StandardLayout::Field field = _recordType.GetField (fieldId);
        assert (field);
        // TODO: Is there any way to add bit support without downgrading overall performance?
        assert (field.GetArchetype () != StandardLayout::FieldArchetype::BIT);
        converted.emplace_back (TrackedZone {field.GetOffset (), field.GetSize ()});
    }

    // In most cases input is already sorted (because it's more convenient to specify fields in order of their
    // declaration), therefore sort should be almost free here.
    std::sort (converted.begin (), converted.end (),
               [] (const TrackedZone &_first, const TrackedZone &_second)
               {
                   return _first.offset < _second.offset;
               });

    Container::InplaceVector<TrackedZone, MAX_COPY_OUT_BLOCKS_PER_EVENT> result;
    result.EmplaceBack (converted.front ());

    if (converted.size () == 1u)
    {
        return result;
    }

    for (auto iterator = std::next (converted.begin ()); iterator != converted.end (); ++iterator)
    {
        if (result.Back ().offset + result.Back ().length == iterator->offset)
        {
            result.Back ().length += iterator->length;
        }
        else
        {
            result.EmplaceBack (*iterator);
        }
    }

    return result;
}

PipelineType GetEventFiringPipeline (EventRoute _route) noexcept
{
    switch (_route)
    {
    case EventRoute::FIXED:
    case EventRoute::FROM_FIXED_TO_NORMAL:
        return PipelineType::FIXED;

    case EventRoute::NORMAL:
        return PipelineType::NORMAL;

    case EventRoute::CUSTOM:
        return PipelineType::CUSTOM;

    case EventRoute::COUNT:
        assert (false);
        return PipelineType::CUSTOM;
    }

    assert (false);
    return PipelineType::CUSTOM;
}

PipelineType GetEventConsumingPipeline (EventRoute _route) noexcept
{
    switch (_route)
    {
    case EventRoute::FIXED:
        return PipelineType::FIXED;

    case EventRoute::NORMAL:
    case EventRoute::FROM_FIXED_TO_NORMAL:
        return PipelineType::NORMAL;

    case EventRoute::CUSTOM:
        return PipelineType::CUSTOM;

    case EventRoute::COUNT:
        assert (false);
        return PipelineType::CUSTOM;
    }

    assert (false);
    return PipelineType::CUSTOM;
}

StandardLayout::Mapping EventTriggerBase::GetTrackedType () const noexcept
{
    return trackedType;
}

StandardLayout::Mapping EventTriggerBase::GetEventType () const noexcept
{
    return inserter.GetTypeMapping ();
}

EventRoute EventTriggerBase::GetRoute () const noexcept
{
    return route;
}

EventTriggerBase::EventTriggerBase (StandardLayout::Mapping _trackedType,
                                    Warehouse::InsertShortTermQuery _inserter,
                                    EventRoute _route) noexcept
    : trackedType (std::move (_trackedType)),
      inserter (std::move (_inserter)),
      route (_route)
{
}

TrivialEventTrigger::TrivialEventTrigger (StandardLayout::Mapping _trackedType,
                                          Warehouse::InsertShortTermQuery _inserter,
                                          EventRoute _route,
                                          const Container::Vector<CopyOutField> &_copyOuts) noexcept
    : EventTriggerBase (std::move (_trackedType), std::move (_inserter), _route),
      copyOuts (BakeCopyOuts (trackedType, _inserter.GetTypeMapping (), _copyOuts))
{
}

void TrivialEventTrigger::Trigger (const void *_record) noexcept
{
    auto cursor = inserter.Execute ();
    void *event = ++cursor;

    for (const CopyOutBlock &block : copyOuts)
    {
        ApplyCopyOut (block, _record, event);
    }
}

OnChangeEventTrigger::OnChangeEventTrigger (StandardLayout::Mapping _trackedType,
                                            Warehouse::InsertShortTermQuery _inserter,
                                            EventRoute _route,
                                            const Container::Vector<StandardLayout::FieldId> &_trackedFields,
                                            const Container::Vector<CopyOutField> &_copyOutOfInitial,
                                            const Container::Vector<CopyOutField> &_copyOutOfChanged) noexcept
    : EventTriggerBase (std::move (_trackedType), std::move (_inserter), _route),
      trackedZones (BakeTrackedFields (_trackedType, _trackedFields)),
      copyOutOfInitial (BakeCopyOuts (trackedType, _inserter.GetTypeMapping (), _copyOutOfInitial)),
      copyOutOfChanged (BakeCopyOuts (trackedType, _inserter.GetTypeMapping (), _copyOutOfChanged))
{
#ifndef _NDEBUG
    for (const CopyOutField &copyOut : _copyOutOfInitial)
    {
        // Only tracked fields can be copied out of unchanged version of record for several reasons:
        // - Requiring old copies of fields, changes in which you do not track, looks strange and bugprone.
        // - Implementation of such behaviour adds unneeded complexity to ChangeTracker baking algorithm.
        assert (std::find (_trackedFields.begin (), _trackedFields.end (), copyOut.recordField) !=
                _trackedFields.end ());
    }
#endif
}

void OnChangeEventTrigger::Trigger (const void *_changedRecord, const void *_initialRecord) noexcept
{
    auto cursor = inserter.Execute ();
    void *event = ++cursor;

    for (const CopyOutBlock &block : copyOutOfInitial)
    {
        ApplyCopyOut (block, _initialRecord, event);
    }

    for (const CopyOutBlock &block : copyOutOfChanged)
    {
        ApplyCopyOut (block, _changedRecord, event);
    }
}

ChangeTracker::ChangeTracker (
    const Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE> &_events) noexcept
    : trackedType (_events[0u]->GetTrackedType ())
{
    for (OnChangeEventTrigger *event : _events)
    {
        for (TrackedZone zone : event->trackedZones)
        {
            for (auto iterator = trackedZones.Begin (); iterator != trackedZones.End (); ++iterator)
            {
                // Cases bellow are illustrated using letters,
                // where A illustrates `zone`, B -- `*iterator`, X -- their intersection.

                /// BBBAAA
                if (zone.offset >= iterator->offset + iterator->length)
                {
                    continue;
                }

                // AAABBB
                if (zone.offset + zone.length <= iterator->offset)
                {
                    trackedZones.EmplaceAt (iterator, zone);
                    zone.length = 0u;
                    break;
                }

                // AAAXXBBB
                if (zone.offset < iterator->offset)
                {
                    std::size_t partLength = iterator->offset - zone.offset;
                    trackedZones.EmplaceAt (iterator, TrackedZone {zone.offset, partLength});
                    ++iterator;

                    zone.offset = iterator->offset;
                    zone.length -= partLength;
                }

                if (zone.offset == iterator->offset)
                {
                    // XXXBBB
                    if (zone.offset + zone.length < iterator->offset + iterator->length)
                    {
                        trackedZones.EmplaceAt (std::next (iterator), TrackedZone {iterator->offset + zone.length,
                                                                                   iterator->length - zone.length});

                        iterator->length = zone.length;
                        zone.length = 0u;
                        break;
                    }

                    // XXXAAA
                    if (zone.offset + zone.length >= iterator->offset + iterator->length)
                    {
                        zone.offset = iterator->offset + iterator->length;
                        zone.length -= iterator->length;
                    }
                }
                // BBBXXX??? -- We split B by begging of A and process intersection with second part in next iteration.
                else
                {
                    std::size_t deltaOffset = zone.offset - iterator->offset;
                    trackedZones.EmplaceAt (std::next (iterator),
                                            TrackedZone {zone.offset, iterator->length - deltaOffset});
                    iterator->length = deltaOffset;
                }
            }

            if (zone.length > 0u)
            {
                trackedZones.EmplaceBack (zone);
            }
        }
    }

#ifndef NDEBUG
    std::size_t totalTrackedSize = 0u;
    for (const TrackedZone &trackerZone : trackedZones)
    {
        totalTrackedSize += trackerZone.length;
    }

    assert (totalTrackedSize <= buffer.size ());
#endif

    for (OnChangeEventTrigger *event : _events)
    {
        EventBinding &binding = bindings.EmplaceBack ();
        binding.event = event;
        decltype (EventBinding::zoneMask) currentZoneFlag = 1u;

        // Calculate binding mask.
        for (const TrackedZone &trackedZone : trackedZones)
        {
            for (const TrackedZone &eventZone : event->trackedZones)
            {
                if (trackedZone.offset >= eventZone.offset &&
                    trackedZone.offset + trackedZone.length <= eventZone.offset + eventZone.length)
                {
                    binding.zoneMask |= currentZoneFlag;
                    break;
                }

                // Assert that there is no intersections: otherwise baking algorithm above is incorrect.
                assert (trackedZone.offset + trackedZone.length < eventZone.offset ||
                        eventZone.offset + eventZone.length < trackedZone.offset);
            }

            currentZoneFlag <<= 1u;
        }

        // Adjust initial copy outs to our buffer offsets.

        // Both are sorted.
        auto copyOutIterator = event->copyOutOfInitial.Begin ();
        auto trackedZoneIterator = trackedZones.Begin ();
        std::size_t bufferOffset = 0u;

        while (copyOutIterator != event->copyOutOfInitial.End ())
        {
            // Buffer just stores tracked data without gaps of untracked data.
            // Copy out of initial points only to tracked data by contract.
            // Therefore, we just need to find offset of the first copy out byte in the buffer.

            while (trackedZoneIterator != trackedZones.End () &&
                   trackedZoneIterator->offset + trackedZoneIterator->length < copyOutIterator->sourceOffset)
            {
                bufferOffset += trackedZoneIterator->offset;
                ++trackedZoneIterator;
            }

            if (trackedZoneIterator == trackedZones.End ())
            {
                // Looks like either "copy out only tracked" contract is broken or blocks aren't sorted properly.
                assert (false);
                break;
            }

            assert (trackedZoneIterator->offset >= copyOutIterator->sourceOffset);
            copyOutIterator->sourceOffset = bufferOffset + copyOutIterator->sourceOffset - trackedZoneIterator->offset;
            ++copyOutIterator;
        }
    }
}

void ChangeTracker::BeginEdition (const void *_record) noexcept
{
    assert (_record);
    uint8_t *bufferOutput = &buffer.front ();

    for (const TrackedZone &zone : trackedZones)
    {
        memcpy (bufferOutput, static_cast<const uint8_t *> (_record) + zone.offset, zone.length);
        bufferOutput += zone.length;
    }
}

void ChangeTracker::EndEdition (const void *_record) noexcept
{
    assert (_record);
    const uint8_t *bufferInput = &buffer.front ();
    decltype (EventBinding::zoneMask) changedMask = 0u;
    decltype (EventBinding::zoneMask) currentZoneFlag = 1u;

    for (const TrackedZone &zone : trackedZones)
    {
        if (memcmp (bufferInput, static_cast<const uint8_t *> (_record) + zone.offset, zone.length) != 0)
        {
            changedMask |= currentZoneFlag;
        }

        currentZoneFlag <<= 1u;
        // TODO: Alignment?
        bufferInput += zone.length;
    }

    for (EventBinding &binding : bindings)
    {
        if (binding.zoneMask & changedMask)
        {
            binding.event->Trigger (_record, &buffer);
        }
    }
}

StandardLayout::Mapping ChangeTracker::GetTrackedType () const noexcept
{
    return trackedType;
}

Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE> ChangeTracker::GetEventTriggers ()
    const noexcept
{
    Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE> triggers;
    for (const EventBinding &binding : bindings)
    {
        triggers.EmplaceBack (binding.event);
    }

    return triggers;
}
} // namespace Celerity

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Celerity::CopyOutField>::Get () noexcept
{
    return DefaultAllocationGroup<StandardLayout::FieldId>::Get ();
}
} // namespace Memory
} // namespace Emergence
