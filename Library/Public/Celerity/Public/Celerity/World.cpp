#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity
{
using namespace Memory::Literals;

static Warehouse::Registry ConstructInsideGroup (Memory::UniqueString _worldName)
{
    auto placeholder = Memory::Profiler::AllocationGroup {_worldName}.PlaceOnTop ();
    return Warehouse::Registry {_worldName};
}

static Memory::Profiler::AllocationGroup WorldAllocationGroup (Memory::UniqueString _worldName,
                                                               Memory::UniqueString _groupName)
{
    return Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup {_worldName}, _groupName};
}

/// It's expected that user will not have a lot of pipelines, therefore no need to waste memory on large pool pages.
static constexpr std::size_t PIPELINES_ON_PAGE = 8u;

World::World (Memory::UniqueString _name, const WorldConfiguration &_configuration) noexcept
    : registry (ConstructInsideGroup (_name)),
      modifyTime (registry.ModifySingleton (TimeSingleton::Reflect ().mapping)),
      modifyWorld (registry.ModifySingleton (WorldSingleton::Reflect ().mapping)),
      pipelinePool (WorldAllocationGroup (_name, "Pipelines"_us), sizeof (Pipeline), PIPELINES_ON_PAGE),
      eventSchemes ({EventScheme {WorldAllocationGroup (_name, "NormalUpdateEventScheme"_us)},
                     EventScheme {WorldAllocationGroup (_name, "FixedUpdateEventScheme"_us)},
                     EventScheme {WorldAllocationGroup (_name, "CustomPipelinesEventScheme"_us)}})
{
    auto timeCursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*timeCursor);
    time->targetFixedFrameDurationsS = _configuration.targetFixedFrameDurationsS;
}

World::~World ()
{
    for (auto iterator = pipelinePool.BeginAcquired (); iterator != pipelinePool.EndAcquired (); ++iterator)
    {
        auto *pipeline = static_cast<Pipeline *> (*iterator);
        pipeline->~Pipeline ();
    }
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

    if (normalPipeline)
    {
        normalPipeline->Execute ();
    }
}

void World::RemovePipeline (Pipeline *_pipeline) noexcept
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
            if (fixedPipeline)
            {
                fixedPipeline->Execute ();
            }

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

        if (fixedPipeline)
        {
            fixedPipeline->Execute ();
        }

        _world->fixedUpdateHappened = true;
        break;
    }
    }
}

Pipeline *World::AddPipeline (Memory::UniqueString _id,
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

    if (_world.normalPipeline)
    {
        _world.normalPipeline->Execute ();
    }

    world->fixedUpdateHappened = false;
}

void WorldTestingUtility::RunFixedUpdateOnce (World &_world) noexcept
{
    auto [time, world] = ExtractSingletons (_world);

    // Keep it simple, because we do not need death spiral avoidance there.
    EMERGENCE_ASSERT (!time->targetFixedFrameDurationsS.Empty ());
    time->fixedDurationS = time->targetFixedFrameDurationsS[0u];
    const auto fixedDurationNs = static_cast<uint64_t> (time->fixedDurationS * 1e9f);

    if (_world.fixedPipeline)
    {
        _world.fixedPipeline->Execute ();
    }

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
