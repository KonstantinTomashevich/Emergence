#include <Shared/Checkpoint.hpp>

#include <Time/TimeSingleton.hpp>
#include <Time/TimeSynchronization.hpp>

namespace TimeSynchronization
{
class FixedTask final
{
public:
    FixedTask (Ogre::Timer *_timer,
               Emergence::Celerity::TaskConstructor &_constructor,
               float _fixedFrameDurationS) noexcept;

    void Execute ();

private:
    Emergence::Warehouse::ModifySingletonQuery modifyTime;
};

FixedTask::FixedTask (Ogre::Timer *_timer,
                      Emergence::Celerity::TaskConstructor &_constructor,
                      float _fixedFrameDurationS) noexcept
    : modifyTime (_constructor.ModifySingleton (TimeSingleton::Reflect ().mapping))
{
    _constructor.MakeDependencyOf (Checkpoint::TIME_UPDATED);

    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);
    time->fixedFrameIndex = 0u;
    time->fixedTimeS = static_cast<float> (_timer->getMilliseconds ()) * 0.001f;
    time->fixedDurationS = _fixedFrameDurationS;
}

void FixedTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);
    ++time->fixedFrameIndex;
    time->fixedTimeS += time->fixedDurationS;
}

class NormalTask final
{
public:
    NormalTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Warehouse::ModifySingletonQuery modifyTime;
    Ogre::Timer *timer;
};

NormalTask::NormalTask (Ogre::Timer *_timer, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyTime (_constructor.ModifySingleton (TimeSingleton::Reflect ().mapping)),
      timer (_timer)
{
    _constructor.MakeDependencyOf (Checkpoint::TIME_UPDATED);

    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);
    time->normalTimeS = static_cast<float> (_timer->getMilliseconds ()) * 0.001f;
}

void NormalTask::Execute ()
{
    auto cursor = modifyTime.Execute ();
    auto *time = static_cast<TimeSingleton *> (*cursor);

    const float currentTimeS = static_cast<float> (timer->getMilliseconds ()) * 0.001f;
    time->normalDurationS = currentTimeS - time->normalTimeS;
    time->normalTimeS = currentTimeS;
}

void AddFixedUpdateTask (Ogre::Timer *_timer,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                         float _fixedFrameDurationS) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("TimeSynchronization::Fixed");
    constructor.SetExecutor (
        [state {FixedTask {_timer, constructor, _fixedFrameDurationS}}] () mutable
        {
            state.Execute ();
        });
}

void AddNormalUpdateTask (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("TimeSynchronization::Normal");
    constructor.SetExecutor (
        [state {NormalTask {_timer, constructor}}] () mutable
        {
            state.Execute ();
        });
}
} // namespace TimeSynchronization
