#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity
{
using namespace Memory::Literals;

template <typename Type, typename... Arguments>
Type ConstructInsideGroup (const Memory::Profiler::AllocationGroup &_group, Arguments... _arguments)
{
    auto placeholder = _group.PlaceOnTop ();
    return Type {std::forward<Arguments> (_arguments)...};
}

static Memory::Profiler::AllocationGroup WorldAllocationGroup (Memory::UniqueString _worldName,
                                                               Memory::UniqueString _groupName)
{
    return Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup {_worldName}, _groupName};
}

Memory::UniqueString WorldView::GetName () const noexcept
{
    return name;
}

void WorldView::RemovePipeline (Pipeline *_pipeline) noexcept
{
    for (auto iterator = pipelinePool.BeginAcquired (); iterator != pipelinePool.EndAcquired (); ++iterator)
    {
        auto *pipeline = static_cast<Pipeline *> (*iterator);
        if (pipeline == _pipeline)
        {
            if (pipeline == normalPipeline)
            {
                normalPipeline = nullptr;
            }

            if (pipeline == fixedPipeline)
            {
                fixedPipeline = nullptr;
            }

            pipeline->~Pipeline ();
            pipelinePool.Release (pipeline);
            return;
        }
    }

    // Received pipeline from another world?
    EMERGENCE_ASSERT (false);
}

WorldView::EventSchemeInstance::EventSchemeInstance (
    const Memory::Profiler::AllocationGroup &_rootAllocationGroup) noexcept
    : onAdd (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnAdd"_us}),
      onRemove (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnRemove"_us}),
      onChange (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnChange"_us})
{
}

/// It's expected that user will not have a lot of pipelines, therefore no need to waste memory on large pool pages.
static constexpr std::size_t PIPELINES_ON_PAGE = 4u;

WorldView::WorldView (World *_world,
                      WorldView *_parent,
                      Memory::UniqueString _name,
                      const WorldViewConfig &_config) noexcept
    : world (_world),
      parent (_parent),
      name (_name),
      config ({Container::HashSet<StandardLayout::Mapping> {Memory::Profiler::AllocationGroup {"EnforcedTypes"_us}}}),

      localRegistry (_name),
      pipelinePool (Memory::Profiler::AllocationGroup {"Pipelines"_us}, sizeof (Pipeline), PIPELINES_ON_PAGE),

      childrenHeap (Memory::Profiler::AllocationGroup {"Children"_us}),
      childrenViews (Memory::Profiler::AllocationGroup::Top ()),
      eventSchemeInstances (
          {EventSchemeInstance {Memory::Profiler::AllocationGroup {"NormalUpdateEventSchemeInstance"_us}},
           EventSchemeInstance {Memory::Profiler::AllocationGroup {"FixedUpdateEventSchemeInstance"_us}},
           EventSchemeInstance {Memory::Profiler::AllocationGroup {"CustomPipelinesEventSchemeInstance"_us}}}),
      eventProductionForbiddenInChildren (Memory::Profiler::AllocationGroup::Top ())
{
    for (const StandardLayout::Mapping &enforcedType : _config.enforcedTypes)
    {
        // Can only be enforced implicitly for root view.
        EMERGENCE_ASSERT (enforcedType != TimeSingleton::Reflect ().mapping);
        EMERGENCE_ASSERT (enforcedType != WorldSingleton::Reflect ().mapping);

        config.enforcedTypes.emplace (enforcedType);
    }

#ifdef EMERGENCE_ASSERT_ENABLED
    // Validate that there is no enforcement overlaps.
    WorldView *viewToCheck = parent;

    while (viewToCheck)
    {
        for (const StandardLayout::Mapping &enforcedType : config.enforcedTypes)
        {
            EMERGENCE_ASSERT (!viewToCheck->config.enforcedTypes.contains (enforcedType));
        }

        viewToCheck = viewToCheck->parent;
    }
#endif
}

WorldView::~WorldView () noexcept
{
    // We need to trigger events that must be inserted into parent views, so they will receive required data.
    // This logic might significantly slow down destruction of the large worlds. Think about improving it later.
    for (EventSchemeInstance &eventSchemeInstance : eventSchemeInstances)
    {
        for (TrivialEventTriggerInstanceRow &row : eventSchemeInstance.onRemove)
        {
            for (TrivialEventTriggerInstance &instance : row)
            {
                const StandardLayout::Mapping &trackedType = instance.GetTrigger ()->GetTrackedType ();
                if (!instance.IsTargetingRegistry (localRegistry) && localRegistry.IsTypeUsed (trackedType))
                {
                    StandardLayout::FieldId idForRangeExtraction = 0u;
                    while (trackedType.GetField (idForRangeExtraction).IsHandleValid () &&
                           trackedType.GetField (idForRangeExtraction).GetArchetype () ==
                               StandardLayout::FieldArchetype::NESTED_OBJECT)
                    {
                        ++idForRangeExtraction;
                    }

                    EMERGENCE_ASSERT (trackedType.GetField (idForRangeExtraction).IsHandleValid ());
                    Warehouse::FetchAscendingRangeQuery query =
                        localRegistry.FetchAscendingRange (trackedType, idForRangeExtraction);
                    auto cursor = query.Execute (nullptr, nullptr);

                    while (const void *record = *cursor)
                    {
                        instance.Trigger (record);
                        ++cursor;
                    }
                }
            }
        }
    }

    for (auto iterator = pipelinePool.BeginAcquired (); iterator != pipelinePool.EndAcquired (); ++iterator)
    {
        auto *pipeline = static_cast<Pipeline *> (*iterator);
        pipeline->~Pipeline ();
    }

    for (WorldView *child : childrenViews)
    {
        child->~WorldView ();
        childrenHeap.Release (child, sizeof (WorldView));
    }
}

Pipeline *WorldView::AddPipeline (Memory::UniqueString _id,
                                  PipelineType _type,
                                  const Task::Collection &_collection) noexcept
{
    EMERGENCE_ASSERT (!normalPipeline || _type != PipelineType::NORMAL);
    EMERGENCE_ASSERT (!fixedPipeline || _type != PipelineType::FIXED);

    auto placeholder = Memory::Profiler::AllocationGroup {pipelinePool.GetAllocationGroup (), _id}.PlaceOnTop ();
    auto *pipeline = new (pipelinePool.Acquire ()) Pipeline {_id, _type, _collection};

    switch (_type)
    {
    case PipelineType::NORMAL:
        normalPipeline = pipeline;
        break;

    case PipelineType::FIXED:
        fixedPipeline = pipeline;
        break;

    case PipelineType::CUSTOM:
        break;

    case PipelineType::COUNT:
        EMERGENCE_ASSERT (false);
        break;
    }

    return pipeline;
}

void WorldView::ExecuteNormalPipeline () noexcept
{
    if (normalPipeline)
    {
        normalPipeline->Execute ();
    }

    for (WorldView *child : childrenViews)
    {
        child->ExecuteNormalPipeline ();
    }
}

void WorldView::ExecuteFixedPipeline () noexcept
{
    if (fixedPipeline)
    {
        fixedPipeline->Execute ();
    }

    for (WorldView *child : childrenViews)
    {
        child->ExecuteFixedPipeline ();
    }
}

WorldView &WorldView::FindViewForType (const StandardLayout::Mapping &_type) noexcept
{
    WorldView *currentView = this;
    while (currentView)
    {
        if (currentView->config.enforcedTypes.contains (_type) || currentView->localRegistry.IsTypeUsed (_type))
        {
            return *currentView;
        }

        currentView = currentView->parent;
    }

    // First occurrence in hierarchy: make type local.
    return *this;
}

TrivialEventTriggerInstanceRow *WorldView::RequestTrivialEventInstances (
    Container::TypedOrderedPool<TrivialEventTriggerInstanceRow> &_pool, const TrivialEventTriggerRow *_source)
{
    EMERGENCE_ASSERT (_source && !_source->Empty ());
    for (TrivialEventTriggerInstanceRow &row : _pool)
    {
        if (!row.Empty () && row.Front ().GetTrigger () == &_source->Front ())
        {
#ifdef EMERGENCE_ASSERT_ENABLED
            EMERGENCE_ASSERT (row.GetCount () == _source->GetCount ());
            for (size_t index = 0u; index < row.GetCount (); ++index)
            {
                EMERGENCE_ASSERT (row[index].GetTrigger () == &(*_source)[index]);
            }
#endif

            return &row;
        }
    }

    TrivialEventTriggerInstanceRow &row = _pool.Acquire ();
    for (const TrivialEventTrigger &trigger : *_source)
    {
        WorldView &view = FindViewForType (trigger.GetEventType ());
        row.EmplaceBack (
            TrivialEventTriggerInstance {&trigger, view.localRegistry.InsertShortTerm (trigger.GetEventType ())});
    }

    return &row;
}

TrivialEventTriggerInstanceRow *WorldView::RequestOnAddEventInstances (PipelineType _pipeline,
                                                                       const TrivialEventTriggerRow *_source) noexcept
{
    return RequestTrivialEventInstances (eventSchemeInstances[static_cast<size_t> (_pipeline)].onAdd, _source);
}

TrivialEventTriggerInstanceRow *WorldView::RequestOnRemoveEventInstances (
    PipelineType _pipeline, const TrivialEventTriggerRow *_source) noexcept
{
    return RequestTrivialEventInstances (eventSchemeInstances[static_cast<size_t> (_pipeline)].onRemove, _source);
}

OnChangeEventTriggerInstanceRow *WorldView::RequestOnChangeEventInstances (PipelineType _pipeline,
                                                                           const ChangeTracker *_source) noexcept
{
    EMERGENCE_ASSERT (_source);
    const ChangeTracker::EventVector eventVector = _source->GetEventTriggers ();
    EMERGENCE_ASSERT (!eventVector.Empty ());

    for (OnChangeEventTriggerInstanceRow &row : eventSchemeInstances[static_cast<size_t> (_pipeline)].onChange)
    {
        if (!row.Empty () && row.Front ().GetTrigger () == eventVector.Front ())
        {
#ifdef EMERGENCE_ASSERT_ENABLED
            EMERGENCE_ASSERT (row.GetCount () == eventVector.GetCount ());
            for (size_t index = 0u; index < row.GetCount (); ++index)
            {
                EMERGENCE_ASSERT (row[index].GetTrigger () == eventVector[index]);
            }
#endif

            return &row;
        }
    }

    OnChangeEventTriggerInstanceRow &row = eventSchemeInstances[static_cast<size_t> (_pipeline)].onChange.Acquire ();
    for (const OnChangeEventTrigger *trigger : eventVector)
    {
        row.EmplaceBack (OnChangeEventTriggerInstance {
            trigger,
            FindViewForType (trigger->GetEventType ()).localRegistry.InsertShortTerm (trigger->GetEventType ())});
    }

    return &row;
}

World::World (Memory::UniqueString _name, const WorldConfiguration &_configuration) noexcept
    : eventSchemes ({EventScheme {WorldAllocationGroup (_name, "NormalUpdateEventScheme"_us)},
                     EventScheme {WorldAllocationGroup (_name, "FixedUpdateEventScheme"_us)},
                     EventScheme {WorldAllocationGroup (_name, "CustomPipelinesEventScheme"_us)}}),
      rootView (ConstructInsideGroup<WorldView> (
          Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup {_name}, "RootView"_us},
          this,
          nullptr,
          "Root"_us,
          _configuration.rootViewConfig)),
      modifyTime (rootView.localRegistry.ModifySingleton (TimeSingleton::Reflect ().mapping)),
      modifyWorld (rootView.localRegistry.ModifySingleton (WorldSingleton::Reflect ().mapping))
{
    auto timeCursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*timeCursor);
    time->targetFixedFrameDurationsS = _configuration.targetFixedFrameDurationsS;
}

const WorldView *World::GetRootView () const noexcept
{
    return &rootView;
}

WorldView *World::GetRootView () noexcept
{
    return &rootView;
}

WorldView *World::CreateView (WorldView *_parent, Memory::UniqueString _name, const WorldViewConfig &_config) noexcept
{
    EMERGENCE_ASSERT (_parent);
    EnsureViewIsOwned (_parent);

    auto placeholder =
        Memory::Profiler::AllocationGroup {_parent->childrenHeap.GetAllocationGroup (), _name}.PlaceOnTop ();

    auto *view = new (_parent->childrenHeap.Acquire (sizeof (WorldView), alignof (WorldView)))
        WorldView (this, _parent, _name, _config);

    _parent->childrenViews.emplace_back (view);
    return view;
}

void World::DropView (WorldView *_view) noexcept
{
    EMERGENCE_ASSERT (_view);
    EnsureViewIsOwned (_view);
    EMERGENCE_ASSERT (_view != &rootView);

    WorldView *parent = _view->parent;
    EMERGENCE_ASSERT (parent);

    auto iterator = std::find (parent->childrenViews.begin (), parent->childrenViews.end (), _view);
    EMERGENCE_ASSERT (iterator != parent->childrenViews.end ());
    // Order matters as it changes update order.
    parent->childrenViews.erase (iterator);

    _view->~WorldView ();
    parent->childrenHeap.Release (_view, sizeof (WorldView));
}

void World::Update () noexcept
{
    TimeSingleton *time = nullptr;
    WorldSingleton *world = nullptr;

    {
        auto timeCursor = modifyTime.Execute ();
        time = static_cast<TimeSingleton *> (*timeCursor);

        auto worldCursor = modifyWorld.Execute ();
        world = static_cast<WorldSingleton *> (*worldCursor);

        // Intentionally release cursors, so pipelines will be able to access singletons.
    }

    // Currently, we're using time->fixed->normal update execution order.
    // It is not ideal solution, but it is the most popular one. Comparison with time->normal->fixed:
    //
    // Pros:
    // - Normal time is always behind fixed time, therefore interpolation never causes tearing.
    //
    // Cons:
    // - Fixed input processing is delayed by one frame.
    // - Unstable fixed updates may cause tearing, because time delta is calculated before fixed update.
    //
    // This is not the final decision: might be changed during next iterations.

    TimeUpdate (time, world);
    FixedUpdate (time, world);
    rootView.ExecuteNormalPipeline ();
}

void World::TimeUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept
{
    // If time delta is higher than 1 seconds, then world update was frozen. It could be because of several reasons:
    // - This is first world update.
    // - World was created and then cached.
    // - Developer stopped program at breakpoint.
    // - Program was frozen for some other reason.
    // In any of these cases we skip one update cycle.
    constexpr uint64_t MAX_TIME_DELTA_NS = 1000000000u;

    const uint64_t currentTimeNs = Emergence::Time::NanosecondsSinceStartup ();
    EMERGENCE_ASSERT (currentTimeNs >= _time->realNormalTimeNs);
    uint64_t realTimeDeltaNs = currentTimeNs - _time->realNormalTimeNs;
    _time->realNormalTimeNs = currentTimeNs;

    if (realTimeDeltaNs > MAX_TIME_DELTA_NS)
    {
        return;
    }

    _time->realNormalDurationS = static_cast<float> (realTimeDeltaNs) * 1e-9f;
    if (_world->fixedUpdateHappened)
    {
        // Previous frame was full update (both normal and fixed pipelines were executed), we need to record its time.
        _time->averageFullFrameRealDurationS.Push (_time->realNormalDurationS);
    }

    EMERGENCE_ASSERT (_time->timeSpeed >= 0.0f);
    float updateModeTimeScale = 1.0f;

    switch (_world->updateMode)
    {
    case WorldUpdateMode::SIMULATING:
        updateModeTimeScale = 1.0f;
        break;
    case WorldUpdateMode::FROZEN:
        updateModeTimeScale = 0.0f;
        break;
    }

    const auto scaledTimeDeltaNs =
        static_cast<uint64_t> (static_cast<float> (realTimeDeltaNs) * updateModeTimeScale * _time->timeSpeed);
    _time->normalDurationS = static_cast<float> (scaledTimeDeltaNs) * 1e-9f;
    _time->normalTimeNs += scaledTimeDeltaNs;
}

void World::FixedUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept
{
    switch (_world->updateMode)
    {
    case WorldUpdateMode::SIMULATING:
    {
        if (_time->fixedTimeNs > _time->normalTimeNs)
        {
            // We are ahead of normal time, no need to do anything.
            _world->fixedUpdateHappened = false;
            return;
        }

        // Adjust fixed update step to avoid death spiral.
        // Full frame time should be consistently lower than simulation step.
        // We also add small epsilon in order to take possible VSync waits into account.
        constexpr float VSYNC_EPSILON = 1e-3f;
        const float minimumRealStepTime = _time->averageFullFrameRealDurationS.Get () - VSYNC_EPSILON;

        EMERGENCE_ASSERT (!_time->targetFixedFrameDurationsS.Empty ());
        std::size_t selectedStepIndex = 0u;

        while (_time->targetFixedFrameDurationsS[selectedStepIndex] < minimumRealStepTime &&
               selectedStepIndex + 1u < _time->targetFixedFrameDurationsS.GetCount ())
        {
            ++selectedStepIndex;
        }

        // We do not need to take time scaling into account,
        // because it affects fixed updates by slowing down normal time.
        _time->fixedDurationS = _time->targetFixedFrameDurationsS[selectedStepIndex];
        const auto fixedDurationNs = static_cast<uint64_t> (_time->fixedDurationS * 1e9f);

        // Catch up to normal time.
        while (_time->fixedTimeNs <= _time->normalTimeNs)
        {
            rootView.ExecuteFixedPipeline ();
            _time->fixedTimeNs += fixedDurationNs;
        }

        _world->fixedUpdateHappened = true;
        break;
    }

    case WorldUpdateMode::FROZEN:
    {
        // Fixed pipeline is executed each frame with zero time
        // step to compensate for changes made by other pipelines.
        _time->fixedDurationS = 0.0f;

        rootView.ExecuteFixedPipeline ();
        _world->fixedUpdateHappened = true;
        break;
    }
    }
}

void World::EnsureViewIsOwned (WorldView *_view) noexcept
{
    if (_view->parent)
    {
        EnsureViewIsOwned (_view->parent);
    }
    else
    {
        EMERGENCE_ASSERT (_view == &rootView);
    }
}

World::EventScheme::EventScheme (const Memory::Profiler::AllocationGroup &_rootAllocationGroup) noexcept
    : custom (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "Custom"_us}),
      onAdd (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnAdd"_us}),
      onRemove (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnRemove"_us}),
      onChange (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "OnChange"_us}),
      changeTrackers (Memory::Profiler::AllocationGroup {_rootAllocationGroup, "ChangeTrackers"_us})
{
}

// TODO: Having separate setup-and-run functions for testing looks a bit bad. Any ideas how to make it better?

void WorldTestingUtility::RunNormalUpdateOnce (World &_world, uint64_t _timeDeltaNs) noexcept
{
    auto [time, world] = ExtractSingletons (_world);
    time->realNormalDurationS = static_cast<float> (_timeDeltaNs) * 1e-9f;

    EMERGENCE_ASSERT (time->timeSpeed >= 0.0f);
    const auto scaledTimeDeltaNs = static_cast<uint64_t> (static_cast<float> (_timeDeltaNs) * time->timeSpeed);

    time->normalDurationS = static_cast<float> (scaledTimeDeltaNs) * 1e-9f;
    time->normalTimeNs += scaledTimeDeltaNs;
    _world.rootView.ExecuteNormalPipeline ();
    world->fixedUpdateHappened = false;
}

void WorldTestingUtility::RunFixedUpdateOnce (World &_world) noexcept
{
    auto [time, world] = ExtractSingletons (_world);

    // Keep it simple, because we do not need death spiral avoidance there.
    EMERGENCE_ASSERT (!time->targetFixedFrameDurationsS.Empty ());
    time->fixedDurationS = time->targetFixedFrameDurationsS[0u];
    const auto fixedDurationNs = static_cast<uint64_t> (time->fixedDurationS * 1e9f);

    _world.rootView.ExecuteFixedPipeline ();
    time->fixedTimeNs += fixedDurationNs;
    world->fixedUpdateHappened = true;
}

std::pair<TimeSingleton *, WorldSingleton *> WorldTestingUtility::ExtractSingletons (World &_world) noexcept
{
    auto timeCursor = _world.modifyTime.Execute ();
    auto worldCursor = _world.modifyWorld.Execute ();
    return {static_cast<TimeSingleton *> (*timeCursor), static_cast<WorldSingleton *> (*worldCursor)};
}
} // namespace Emergence::Celerity
