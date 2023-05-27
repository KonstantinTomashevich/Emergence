#include <Container/Vector.hpp>

#include <Job/Dispatcher.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>
#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Job
{
using namespace Memory::Literals;

class DispatcherImplementation final
{
public:
    DispatcherImplementation (std::size_t _threadCount) noexcept;

    DispatcherImplementation (const DispatcherImplementation &_other) = delete;

    DispatcherImplementation (DispatcherImplementation &&_other) = delete;

    ~DispatcherImplementation () noexcept;

    void Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept;

    [[nodiscard]] std::size_t GetAvailableThreadsCount () const noexcept;

    void SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DispatcherImplementation);

private:
    Dispatcher::Job Pop (bool &_wasBackground) noexcept;

    Container::Vector<std::jthread> threads {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};
    std::size_t maxBackgroundThreadCount;

    Container::Vector<Dispatcher::Job> foregroundJobPool {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};
    Container::Vector<Dispatcher::Job> backgroundJobPool {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};

    std::atomic_flag modifyingPool;
    std::atomic_flag hasJobs;

    std::atomic_flag terminating;

    std::atomic_uintptr_t availableThreadsCount = 0u;
    std::atomic_uintptr_t backgroundThreadCount = 0u;
};

DispatcherImplementation::DispatcherImplementation (std::size_t _threadCount) noexcept
    : maxBackgroundThreadCount (_threadCount / 2u)
{
    foregroundJobPool.reserve (32u);
    backgroundJobPool.reserve (32u);

    for (std::size_t threadIndex = 0u; threadIndex < _threadCount; ++threadIndex)
    {
        threads.emplace_back (
            [this] ()
            {
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
                    job ();

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
    hasJobs.test_and_set (std::memory_order_release);
    hasJobs.notify_all ();

    for (std::jthread &thread : threads)
    {
        thread.join ();
    }
}

void DispatcherImplementation::Dispatch (Priority _jobPriority, Dispatcher::Job _job) noexcept
{
    AtomicFlagGuard guard {modifyingPool};
    switch (_jobPriority)
    {
    case Priority::FOREGROUND:
        foregroundJobPool.emplace_back (std::move (_job));
        break;

    case Priority::BACKGROUND:
        backgroundJobPool.emplace_back (std::move (_job));
        break;
    }

    hasJobs.test_and_set (std::memory_order_release);
    hasJobs.notify_one ();
}

std::size_t DispatcherImplementation::GetAvailableThreadsCount () const noexcept
{
    return availableThreadsCount;
}

void DispatcherImplementation::SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept
{
    maxBackgroundThreadCount = _maxBackgroundThreadCount;
}

Dispatcher::Job DispatcherImplementation::Pop (bool &_wasBackground) noexcept
{
    while (true)
    {
        hasJobs.wait (false, std::memory_order_acquire);
        AtomicFlagGuard guard {modifyingPool};

        if (terminating.test (std::memory_order_acquire))
        {
            // Empty job to schedule termination.
            _wasBackground = false;
            return [] ()
            {
            };
        }

        if (foregroundJobPool.empty () && backgroundJobPool.empty ())
        {
            continue;
        }

        Dispatcher::Job job;
        if (!backgroundJobPool.empty () && backgroundThreadCount < maxBackgroundThreadCount)
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

        if (foregroundJobPool.empty () && backgroundJobPool.empty ())
        {
            hasJobs.clear (std::memory_order_release);
        }

        return job;
    }
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
