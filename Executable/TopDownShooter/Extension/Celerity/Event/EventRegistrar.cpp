#include <cassert>

#include <Celerity/Event/EventRegistrar.hpp>

#include <Container/HashMap.hpp>

namespace Emergence::Celerity
{
EventRegistrar::EventRegistrar (World *_world) noexcept : world (_world)
{
    assert (world);
    // There should be no pipelines, because events augment pipeline building process.
    assert (world->pipelinePool.BeginAcquired () == world->pipelinePool.EndAcquired ());

    world->eventOnAdd.clear ();
    world->eventOnRemove.clear ();
    world->eventOnChange.clear ();
    world->changeTrackers.clear ();
    world->eventCustom.clear ();
}

EventRegistrar::EventRegistrar (EventRegistrar &&_other) noexcept : world (_other.world)
{
    _other.world = nullptr;
}

EventRegistrar::~EventRegistrar () noexcept
{
    if (world)
    {
        using EventVector = Container::InplaceVector<OnChangeEventTrigger *, MAX_ON_CHANGE_EVENTS_PER_TYPE>;
        static Memory::Profiler::AllocationGroup allocationGroup {Memory::Profiler::AllocationGroup::Root (),
                                                                  Memory::UniqueString {"EventRegistrationAlgorithms"}};

        Container::HashMap<StandardLayout::Mapping, EventVector> onChangeEventPerType {allocationGroup};
        for (OnChangeEventTrigger &trigger : world->eventOnChange)
        {
            auto iterator = onChangeEventPerType.find (trigger.GetTrackedType ());
            if (iterator == onChangeEventPerType.end ())
            {
                iterator = onChangeEventPerType.emplace (trigger.GetTrackedType (), EventVector {}).first;
            }

            [[maybe_unused]] bool inserted = iterator->second.TryEmplaceBack (&trigger);
            assert (inserted);
        }

        world->changeTrackers.reserve (onChangeEventPerType.size ());
        for (auto &[trackedType, events] : onChangeEventPerType)
        {
            world->changeTrackers.emplace_back (ChangeTracker {events});
        }

        world->eventOnAdd.shrink_to_fit ();
        world->eventOnRemove.shrink_to_fit ();
        world->eventOnChange.shrink_to_fit ();
        world->eventCustom.shrink_to_fit ();
    }
}

void EventRegistrar::CustomEvent (const ClearableEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    world->eventCustom.emplace_back (World::CustomEventInfo {_seed.eventType, _seed.route});
}

static void AddTrivialAutomatedEvent (Container::Vector<TrivialEventTriggerRow> &_target,
                                      const TrivialAutomatedEventSeed &_seed,
                                      Warehouse::Registry &_registry)
{
    TrivialEventTriggerRow *selectedRow = nullptr;
    for (TrivialEventTriggerRow &row : _target)
    {
        if (row.Empty () || row.Front ().GetTrackedType () == _seed.trackedType)
        {
            selectedRow = &row;
            break;
        }
    }

    if (!selectedRow)
    {
        selectedRow = &_target.emplace_back ();
    }

    [[maybe_unused]] bool inserted = selectedRow->TryEmplaceBack (TrivialEventTrigger {
        _seed.trackedType, _registry.InsertShortTerm (_seed.eventType), _seed.route, _seed.copyOut});
    assert (inserted);
}

void EventRegistrar::OnAddEvent (const TrivialAutomatedEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    AddTrivialAutomatedEvent (world->eventOnAdd, _seed, world->registry);
}

void EventRegistrar::OnRemoveEvent (const TrivialAutomatedEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    AddTrivialAutomatedEvent (world->eventOnRemove, _seed, world->registry);
}

void EventRegistrar::OnChangeEvent (const OnChangeAutomatedEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    world->eventOnChange.emplace_back (
        OnChangeEventTrigger {_seed.trackedType, world->registry.InsertShortTerm (_seed.eventType), _seed.route,
                              _seed.trackedFields, _seed.copyOutOfInitial, _seed.copyOutOfChanged});
}

void EventRegistrar::AssertEventUniqueness ([[maybe_unused]] const StandardLayout::Mapping &_type) const noexcept
{
#ifndef NDEBUG
    assert (world);

    for (const TrivialEventTriggerRow &row : world->eventOnAdd)
    {
        for (const TrivialEventTrigger &trigger : row)
        {
            assert (trigger.GetEventType () != _type);
        }
    }

    for (const TrivialEventTriggerRow &row : world->eventOnRemove)
    {
        for (const TrivialEventTrigger &trigger : row)
        {
            assert (trigger.GetEventType () != _type);
        }
    }

    for (const OnChangeEventTrigger &trigger : world->eventOnChange)
    {
        assert (trigger.GetEventType () != _type);
    }

    for (const World::CustomEventInfo &info : world->eventCustom)
    {
        assert (info.type != _type);
    }
#endif
}
} // namespace Emergence::Celerity
