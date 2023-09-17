#include <API/Common/BlockCast.hpp>

#include <Container/Vector.hpp>

#include <CPU/Profiler.hpp>

#include <Job/Dispatcher.hpp>

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence::Job
{
using namespace Memory::Literals;

class DispatcherImplementation final
{
public:
    class Batch final
    {
    public:
        Batch (DispatcherImplementation *_owner) noexcept;

        Batch (const Batch &_other) = delete;

        Batch (Batch &&_other) = delete;

        ~Batch () noexcept = default;

        void Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Batch);

    private:
        DispatcherImplementation *owner;
        AtomicFlagGuard poolModificationGuard;
    };

    DispatcherImplementation (std::size_t _threadCount) noexcept;

    DispatcherImplementation (const DispatcherImplementation &_other) = delete;

    DispatcherImplementation (DispatcherImplementation &&_other) = delete;

    ~DispatcherImplementation () noexcept;

    void Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept;

    [[nodiscard]] std::size_t GetAvailableThreadsCount () const noexcept;

    void SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DispatcherImplementation);

private:
    friend class Batch;

    void DispatchInternal (Priority _jobPriority, Dispatcher::Job _job) noexcept;

    Dispatcher::Job Pop (bool &_wasBackground) noexcept;

    Container::Vector<std::jthread> threads {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};
    std::size_t maxBackgroundThreadCount;

    Container::Vector<Dispatcher::Job> foregroundJobPool {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};
    Container::Vector<Dispatcher::Job> backgroundJobPool {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};

    std::atomic_flag modifyingPool;
    std::atomic_uintptr_t jobCounter;

    std::atomic_flag terminating;

    std::atomic_uintptr_t availableThreadsCount = 0u;
    std::atomic_uintptr_t backgroundThreadCount = 0u;
};

DispatcherImplementation::Batch::Batch (DispatcherImplementation *_owner) noexcept
    : owner (_owner),
      poolModificationGuard (owner->modifyingPool)
{
}

void DispatcherImplementation::Batch::Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    owner->DispatchInternal (_jobPriority, std::move (_job));
}

DispatcherImplementation::DispatcherImplementation (std::size_t _threadCount) noexcept
    : maxBackgroundThreadCount (_threadCount / 2u)
{
    foregroundJobPool.reserve (32u);
    backgroundJobPool.reserve (32u);

    static const Memory::UniqueString threadName {"JobDispatcherThread"};
    static CPU::Profiler::SectionDefinition foregroundJobSection {*"ForegroundJob"_us, 0xFF999900u};
    static CPU::Profiler::SectionDefinition backgroundJobSection {*"BackgroundJob"_us, 0xFF999900u};

    for (std::size_t threadIndex = 0u; threadIndex < _threadCount; ++threadIndex)
    {
        threads.emplace_back (
            [this] ()
            {
                CPU::Profiler::SetThreadName (*threadName);
                while (true)
                {
                    ++availableThreadsCount;
                    if (terminating.test (std::memory_order_acquire))
                    {
                        return;
                    }

                    bool wasBackground;
                    Dispatcher::Job job = Pop (wasBackground);
                    --availableThreadsCount;

                    {
                        CPU::Profiler::SectionInstance section {wasBackground ? backgroundJobSection :
                                                                                foregroundJobSection};
                        job ();
                    }

                    if (wasBackground)
                    {
                        --backgroundThreadCount;
                    }
                }
            });
    }
}

DispatcherImplementation::~DispatcherImplementation () noexcept
{
    terminating.test_and_set (std::memory_order_acquire);
    jobCounter.store (1u, std::memory_order_release);
    jobCounter.notify_all ();

    for (std::jthread &thread : threads)
    {
        thread.join ();
    }
}

void DispatcherImplementation::Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    static CPU::Profiler::SectionDefinition dispatchSection {*"JobDispatcherDispatch"_us, 0xFF990000u};
    CPU::Profiler::SectionInstance section {dispatchSection};

    AtomicFlagGuard guard {modifyingPool};
    DispatchInternal (_jobPriority, std::move (_job));
}

std::size_t DispatcherImplementation::GetAvailableThreadsCount () const noexcept
{
    return availableThreadsCount;
}

void DispatcherImplementation::SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept
{
    maxBackgroundThreadCount = _maxBackgroundThreadCount;
}

void DispatcherImplementation::DispatchInternal (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    static CPU::Profiler::SectionDefinition dispatchInternalSection {*"JobDispatcherDispatchInternal"_us, 0xFF000099u};
    CPU::Profiler::SectionInstance section {dispatchInternalSection};

    switch (_jobPriority)
    {
    case Priority::FOREGROUND:
        foregroundJobPool.emplace_back (std::move (_job));
        break;

    case Priority::BACKGROUND:
        backgroundJobPool.emplace_back (std::move (_job));
        break;
    }

    jobCounter.fetch_add (1u, std::memory_order_release);
    jobCounter.notify_one ();
}

Dispatcher::Job DispatcherImplementation::Pop (bool &_wasBackground) noexcept
{
    static CPU::Profiler::SectionDefinition popSection {*"JobDispatcherPop"_us, 0xFF990000u};
    CPU::Profiler::SectionInstance section {popSection};

    while (true)
    {
        jobCounter.wait (0u, std::memory_order_acquire);
        AtomicFlagGuard guard {modifyingPool};

        if (terminating.test (std::memory_order_acquire))
        {
            // Empty job to schedule termination.
            _wasBackground = false;
            return [] ()
            {
            };
        }

        const bool noForegroundJobs = foregroundJobPool.empty ();
        const bool noBackgroundJobs = backgroundJobPool.empty ();
        const bool underTheBackgroundJobLimit = backgroundThreadCount < maxBackgroundThreadCount;

        if ((noBackgroundJobs || !underTheBackgroundJobLimit) && noForegroundJobs)
        {
            continue;
        }

        Dispatcher::Job job;
        if (!noBackgroundJobs && underTheBackgroundJobLimit)
        {
            job = std::move (backgroundJobPool.back ());
            backgroundJobPool.pop_back ();
            ++backgroundThreadCount;
            _wasBackground = true;
        }
        else
        {
            job = std::move (foregroundJobPool.back ());
            foregroundJobPool.pop_back ();
            _wasBackground = false;
        }

        jobCounter.fetch_sub (1u, std::memory_order_release);
        return job;
    }
}

Dispatcher::Batch::Batch (Dispatcher &_dispatcher) noexcept
{
    new (&data) DispatcherImplementation::Batch (&block_cast<DispatcherImplementation> (_dispatcher.data));
}

Dispatcher::Batch::~Batch () noexcept
{
    block_cast<DispatcherImplementation::Batch> (data).~Batch ();
}

void Dispatcher::Batch::Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    block_cast<DispatcherImplementation::Batch> (data).Dispatch (_jobPriority, std::move (_job));
}

Dispatcher &Dispatcher::Global () noexcept
{
    static Dispatcher globalDispatcher {std::thread::hardware_concurrency ()};
    return globalDispatcher;
}

Dispatcher::Dispatcher (std::size_t _threadCount) noexcept
{
    new (&data) DispatcherImplementation (_threadCount);
}

Dispatcher::~Dispatcher () noexcept
{
    block_cast<DispatcherImplementation> (data).~DispatcherImplementation ();
}

void Dispatcher::Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    block_cast<DispatcherImplementation> (data).Dispatch (_jobPriority, std::move (_job));
}

std::size_t Dispatcher::GetAvailableThreadsCount () const noexcept
{
    return block_cast<DispatcherImplementation> (data).GetAvailableThreadsCount ();
}

void Dispatcher::SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept
{
    block_cast<DispatcherImplementation> (data).SetMaximumThreadsForBackgroundExecution (_maxBackgroundThreadCount);
}
} // namespace Emergence::Job
