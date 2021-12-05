#include <Shared/Checkpoint.hpp>

#include <Time/TimeSingleton.hpp>
#include <Time/TimeSynchronization.hpp>

namespace TimeSynchronization
{
namespace
{
class TaskBase
{
public:
    TaskBase (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

protected:
    Emergence::Warehouse::ModifySingletonQuery modifyTime;
    Ogre::Timer *timer;
};

class FixedSyncTask final : public TaskBase
{
public:
    FixedSyncTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();
};

class FixedDurationTask final : public TaskBase
{
public:
    FixedDurationTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();
};

class NormalSyncTask final : public TaskBase
{
public:
    NormalSyncTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();
};

class NormalDurationTask final : public TaskBase
{
public:
    NormalDurationTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();
};

TaskBase::TaskBase (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyTime (_constructor.ModifySingleton (TimeSingleton::Reflect ().mapping)),
      timer (_timer)
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    if (!time->constructed)
    {
        new (time) TimeSingleton ();
    }
}

FixedSyncTask::FixedSyncTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskBase (_timer, _constructor)
{
    _constructor.MakeDependencyOf (Checkpoint::TIME_UPDATED);
    _constructor.MakeDependencyOf (Checkpoint::FRAME_STATS_COLLECTION);
}

void FixedSyncTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    time->fixedStartUs = timer->getMicroseconds ();

    // We assume that there is either no dependency between simulation step duration and real fixed update duration
    // or that this dependency is very small.
    const float averageFixed = time->averageFixedRealDurationS.Get ();
    const float averageNormal = time->averageNormalRealDurationS.Get ();

    // To avoid death spiral, we must do at least one fixed and one normal update for each simulation step.
    const float minimumRealStepTime = averageFixed + averageNormal;

    assert (!time->targetFixedFrameDurationsS.Empty ());
    std::size_t selectedStepIndex = 0u;

    while (time->targetFixedFrameDurationsS[selectedStepIndex] < minimumRealStepTime &&
           selectedStepIndex + 1u < time->targetFixedFrameDurationsS.GetCount ())
    {
        ++selectedStepIndex;
    }

    time->fixedDurationS = time->targetFixedFrameDurationsS[selectedStepIndex];
    time->fixedTimeUs += static_cast<uint64_t> (time->fixedDurationS * 1e6f);
}

FixedDurationTask::FixedDurationTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskBase (_timer, _constructor)
{
    _constructor.DependOn (Checkpoint::FRAME_STATS_COLLECTION);
}

void FixedDurationTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    const uint64_t frameDurationUs = timer->getMicroseconds () - time->fixedStartUs;
    const float frameDurationS = static_cast<float> (frameDurationUs) * 1e-6f;

    if (frameDurationS < 1.0f)
    {
        time->averageFixedRealDurationS.Push (frameDurationS);
    }
    else
    {
        // Looks like we were sitting on breakpoint. Just skip this frame then.
    }
}

NormalSyncTask::NormalSyncTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskBase (_timer, _constructor)
{
    _constructor.MakeDependencyOf (Checkpoint::TIME_UPDATED);
    _constructor.MakeDependencyOf (Checkpoint::FRAME_STATS_COLLECTION);

    auto cursor = modifyTime.Execute ();
    static_cast<TimeSingleton *> (*cursor)->normalTimeOffsetUs = timer->getMicroseconds ();
}

void NormalSyncTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    const uint64_t currentTimeUs = timer->getMicroseconds () - time->normalTimeOffsetUs;
    time->normalDurationS = static_cast<float> (currentTimeUs - time->normalTimeUs) * 1e-6f;
    time->normalTimeUs = currentTimeUs;
}

NormalDurationTask::NormalDurationTask (Ogre::Timer *_timer,
                                        Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskBase (_timer, _constructor)
{
    _constructor.DependOn (Checkpoint::FRAME_STATS_COLLECTION);
}

void NormalDurationTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    const uint64_t localizedTimeUs = timer->getMicroseconds () - time->normalTimeOffsetUs;
    const uint64_t frameDurationUs = localizedTimeUs - time->normalTimeUs;
    const float frameDurationS = static_cast<float> (frameDurationUs) * 1e-6f;

    if (frameDurationS < 1.0f)
    {
        time->averageNormalRealDurationS.Push (frameDurationS);
    }
    else
    {
        // Looks like we were sitting on breakpoint. Just skip this frame then.
    }
}
} // namespace

void AddFixedUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("TimeSynchronization::SyncFixed");
    constructor.SetExecutor (
        [state {FixedSyncTask {_timer, constructor}}] () mutable
        {
            state.Execute ();
        });

    constructor = _pipelineBuilder.AddTask ("TimeSynchronization::StatsFixed");
    constructor.SetExecutor (
        [state {FixedDurationTask {_timer, constructor}}] () mutable
        {
            state.Execute ();
        });
}

void AddNormalUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("TimeSynchronization::SyncNormal");
    constructor.SetExecutor (
        [state {NormalSyncTask {_timer, constructor}}] () mutable
        {
            state.Execute ();
        });

    constructor = _pipelineBuilder.AddTask ("TimeSynchronization::StatsNormal");
    constructor.SetExecutor (
        [state {NormalDurationTask {_timer, constructor}}] () mutable
        {
            state.Execute ();
        });
}
} // namespace TimeSynchronization