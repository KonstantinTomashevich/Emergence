#include <Job/Dispatcher.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>
#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Job
{
class DispatcherImplementation final
{
public:
    DispatcherImplementation (std::size_t _threadCount) noexcept;

    DispatcherImplementation (const DispatcherImplementation &_other) = delete;

    DispatcherImplementation (DispatcherImplementation &&_other) = delete;

    ~DispatcherImplementation () noexcept;

    void Dispatch (Dispatcher::Job _job) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DispatcherImplementation);

private:
    Dispatcher::Job Pop () noexcept;

    std::vector<std::jthread> threads;

    std::vector<Dispatcher::Job> jobPool;

    std::atomic_flag modifyingPool;
    std::atomic_flag hasJobs;

    std::atomic_flag terminating;
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
                    if (terminating.test (std::memory_order_acquire))
                    {
                        return;
                    }

                    Pop () ();
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
    while (true)
    {
        AtomicFlagGuard guard {modifyingPool};
        jobPool.emplace_back (std::move (_job));
        hasJobs.test_and_set (std::memory_order_release);
        hasJobs.notify_one ();
        return;
    }
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
} // namespace Emergence::Job
