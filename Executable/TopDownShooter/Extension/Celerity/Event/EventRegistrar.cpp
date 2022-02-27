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

    // In case we are reusing world and there are old schemes.
    for (World::EventScheme &scheme : world->eventSchemes)
    {
        scheme.custom.clear ();
        scheme.onAdd.clear ();
        scheme.onRemove.clear ();
        scheme.onChange.clear ();
        scheme.changeTrackers.clear ();
    }
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
        for (World::EventScheme &scheme : world->eventSchemes)
        {
            for (OnChangeEventTrigger &trigger : scheme.onChange)
            {
                auto iterator = onChangeEventPerType.find (trigger.GetTrackedType ());
                if (iterator == onChangeEventPerType.end ())
                {
                    iterator = onChangeEventPerType.emplace (trigger.GetTrackedType (), EventVector {}).first;
                }

                [[maybe_unused]] bool inserted = iterator->second.TryEmplaceBack (&trigger);
                assert (inserted);
            }

            scheme.changeTrackers.reserve (onChangeEventPerType.size ());
            for (auto &[trackedType, events] : onChangeEventPerType)
            {
                scheme.changeTrackers.emplace_back (events);
            }

            onChangeEventPerType.clear ();
            scheme.custom.shrink_to_fit ();
            scheme.onAdd.shrink_to_fit ();
            scheme.onRemove.shrink_to_fit ();
            scheme.onChange.shrink_to_fit ();
        }
    }
}

void EventRegistrar::CustomEvent (const ClearableEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    SelectScheme (_seed.route).custom.emplace_back (World::CustomEventInfo {_seed.eventType, _seed.route});
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
    AddTrivialAutomatedEvent (SelectScheme (_seed.route).onAdd, _seed, world->registry);
}

void EventRegistrar::OnRemoveEvent (const TrivialAutomatedEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);
    AddTrivialAutomatedEvent (SelectScheme (_seed.route).onRemove, _seed, world->registry);
}

void EventRegistrar::OnChangeEvent (const OnChangeAutomatedEventSeed &_seed) noexcept
{
    assert (world);
    AssertEventUniqueness (_seed.eventType);

    OnChangeEventTrigger trigger {_seed.trackedType,
                                  world->registry.InsertShortTerm (_seed.eventType),
                                  _seed.route,
                                  _seed.trackedFields,
                                  _seed.copyOutOfInitial,
                                  _seed.copyOutOfChanged};

    SelectScheme (_seed.route).onChange.emplace_back (std::move (trigger));
}

void EventRegistrar::AssertEventUniqueness ([[maybe_unused]] const StandardLayout::Mapping &_type) const noexcept
{
#ifndef NDEBUG
    assert (world);

    for (World::EventScheme &scheme : world->eventSchemes)
    {
        for (const World::CustomEventInfo &info : scheme.custom)
        {
            assert (info.type != _type);
        }

        for (const TrivialEventTriggerRow &row : scheme.onAdd)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                assert (trigger.GetEventType () != _type);
            }
        }

        for (const TrivialEventTriggerRow &row : scheme.onRemove)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                assert (trigger.GetEventType () != _type);
            }
        }

        for (const OnChangeEventTrigger &trigger : scheme.onChange)
        {
            assert (trigger.GetEventType () != _type);
        }
    }
#endif
}

World::EventScheme &EventRegistrar::SelectScheme (EventRoute _route) noexcept
{
    return world->eventSchemes[static_cast<std::size_t> (GetEventProducingPipeline (_route))];
}
} // namespace Emergence::Celerity
