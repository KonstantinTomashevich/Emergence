#include <Celerity/Event/EventRegistrar.hpp>

#include <Container/HashMap.hpp>

namespace Emergence::Celerity
{
EventRegistrar::EventRegistrar (World *_world, [[maybe_unused]] bool unsafe) noexcept
    : world (_world)
{
    EMERGENCE_ASSERT (world);
    // There should be no pipelines and no child views, because events augment pipeline building process.
    // If unsafe was passed, we think that user is responsible for recreating all the pipelines.
    EMERGENCE_ASSERT (unsafe ||
                      (world->rootView.pipelinePool.BeginAcquired () == world->rootView.pipelinePool.EndAcquired () &&
                       world->rootView.childrenViews.empty ()));

    // In case we are reusing world and there are old schemes.
    for (World::EventScheme &scheme : world->eventSchemes)
    {
        scheme.custom.Clear ();
        scheme.onAdd.Clear ();
        scheme.onRemove.Clear ();
        scheme.onChange.Clear ();
        scheme.changeTrackers.Clear ();
    }
}

EventRegistrar::EventRegistrar (EventRegistrar &&_other) noexcept
    : world (_other.world)
{
    _other.world = nullptr;
}

EventRegistrar::~EventRegistrar () noexcept
{
    if (world)
    {
        // We need to group on change event triggers into change trackers.
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
                EMERGENCE_ASSERT (inserted);
            }

            for (auto &[trackedType, events] : onChangeEventPerType)
            {
                scheme.changeTrackers.Acquire (events);
            }

            onChangeEventPerType.clear ();
            scheme.custom.Shrink ();
            scheme.onAdd.Shrink ();
            scheme.onRemove.Shrink ();
            scheme.onChange.Shrink ();
        }
    }
}

void EventRegistrar::CustomEvent (const ClearableEventSeed &_seed) noexcept
{
    EMERGENCE_ASSERT (world);
    AssertEventUniqueness (_seed.eventType);
    SelectScheme (_seed.route).custom.Acquire (World::CustomEventInfo {_seed.eventType, _seed.route});
}

static void AddTrivialAutomatedEvent (Container::TypedOrderedPool<TrivialEventTriggerRow> &_target,
                                      const TrivialAutomatedEventSeed &_seed)
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
        selectedRow = &_target.Acquire ();
    }

    [[maybe_unused]] bool inserted = selectedRow->TryEmplaceBack (
        TrivialEventTrigger {_seed.trackedType, _seed.eventType, _seed.route, _seed.copyOut});
    EMERGENCE_ASSERT (inserted);
}

void EventRegistrar::OnAddEvent (const TrivialAutomatedEventSeed &_seed) noexcept
{
    EMERGENCE_ASSERT (world);
    AssertEventUniqueness (_seed.eventType);
    AddTrivialAutomatedEvent (SelectScheme (_seed.route).onAdd, _seed);
}

void EventRegistrar::OnRemoveEvent (const TrivialAutomatedEventSeed &_seed) noexcept
{
    EMERGENCE_ASSERT (world);
    AssertEventUniqueness (_seed.eventType);
    AddTrivialAutomatedEvent (SelectScheme (_seed.route).onRemove, _seed);
}

void EventRegistrar::OnChangeEvent (const OnChangeAutomatedEventSeed &_seed) noexcept
{
    EMERGENCE_ASSERT (world);
    AssertEventUniqueness (_seed.eventType);

    OnChangeEventTrigger trigger {_seed.trackedType,   _seed.eventType,        _seed.route,
                                  _seed.trackedFields, _seed.copyOutOfInitial, _seed.copyOutOfChanged};

    SelectScheme (_seed.route).onChange.Acquire (std::move (trigger));
}

void EventRegistrar::AssertEventUniqueness ([[maybe_unused]] const StandardLayout::Mapping &_type) const noexcept
{
#if defined(EMERGENCE_ASSERT_ENABLED)
    EMERGENCE_ASSERT (world);

    for (World::EventScheme &scheme : world->eventSchemes)
    {
        for (const World::CustomEventInfo &info : scheme.custom)
        {
            EMERGENCE_ASSERT (info.type != _type);
        }

        for (const TrivialEventTriggerRow &row : scheme.onAdd)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                EMERGENCE_ASSERT (trigger.GetEventType () != _type);
            }
        }

        for (const TrivialEventTriggerRow &row : scheme.onRemove)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                EMERGENCE_ASSERT (trigger.GetEventType () != _type);
            }
        }

        for (const OnChangeEventTrigger &trigger : scheme.onChange)
        {
            EMERGENCE_ASSERT (trigger.GetEventType () != _type);
        }
    }
#endif
}

World::EventScheme &EventRegistrar::SelectScheme (EventRoute _route) noexcept
{
    return world->eventSchemes[static_cast<std::size_t> (GetEventProducingPipeline (_route))];
}
} // namespace Emergence::Celerity
