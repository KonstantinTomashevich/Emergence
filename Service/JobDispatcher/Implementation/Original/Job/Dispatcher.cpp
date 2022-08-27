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

    void Dispatch (Dispatcher::Job _job) noexcept;

    [[nodiscard]] std::size_t GetAvailableThreadsCount () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DispatcherImplementation);

private:
    Dispatcher::Job Pop () noexcept;

    Container::Vector<std::jthread> threads {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};

    Container::Vector<Dispatcher::Job> jobPool {Memory::Profiler::AllocationGroup {"JobDispatcher"_us}};

    std::atomic_flag modifyingPool;
    std::atomic_flag hasJobs;

    std::atomic_flag terminating;

    std::atomic_unsigned_lock_free availableThreadsCount = 0u;
};

DispatcherImplementation::DispatcherImplementation (std::size_t _threadCount) noexcept
{
    jobPool.reserve (32u);
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

                    Dispatcher::Job job = Pop ();
                    --availableThreadsCount;
                    job ();
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

void DispatcherImplementation::Dispatch (Dispatcher::Job _job) noexcept
{
    AtomicFlagGuard guard {modifyingPool};
    jobPool.emplace_back (std::move (_job));
    hasJobs.test_and_set (std::memory_order_release);
    hasJobs.notify_one ();
}

std::size_t DispatcherImplementation::GetAvailableThreadsCount () const noexcept
{
    return availableThreadsCount;
}

Dispatcher::Job DispatcherImplementation::Pop () noexcept
{
    while (true)
    {
        hasJobs.wait (false, std::memory_order_acquire);
        AtomicFlagGuard guard {modifyingPool};

        if (terminating.test (std::memory_order_acquire))
        {
            // Empty job to schedule termination.
            return [] ()
            {
            };
        }

        if (jobPool.empty ())
        {
            continue;
        }

        Dispatcher::Job job {std::move (jobPool.back ())};
        jobPool.pop_back ();

        if (jobPool.empty ())
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

void Dispatcher::Dispatch (Dispatcher::Job _job) noexcept
{
    block_cast<DispatcherImplementation> (data).Dispatch (std::move (_job));
}

std::size_t Dispatcher::GetAvailableThreadsCount () const noexcept
{
    return block_cast<DispatcherImplementation> (data).GetAvailableThreadsCount ();
}
} // namespace Emergence::Job
