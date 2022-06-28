#include <atomic>
#include <cassert>
#include <thread>

#include <Container/Vector.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>
#include <SyntaxSugar/BlockCast.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Task
{
// TODO: Make job executor separate library, so it can be used for other jobs, like PhysX CPU dispatch?
class JobExecutor final
{
public:
    using Job = std::function<void ()>;

    JobExecutor () noexcept;

    JobExecutor (const JobExecutor &_other) = delete;

    JobExecutor (JobExecutor &&_other) = delete;

    ~JobExecutor () noexcept;

    void Push (Job _task) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (JobExecutor);

private:
    Job Pop () noexcept;

    std::vector<std::jthread> threads;

    std::vector<Job> jobPool;

    std::atomic_flag modifyingPool;
    std::atomic_flag hasJobs;

    std::atomic_flag terminating;
};

JobExecutor::JobExecutor () noexcept
{
    jobPool.reserve (32u);
    for (std::size_t threadIndex = 0u; threadIndex < std::thread::hardware_concurrency (); ++threadIndex)
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

JobExecutor::~JobExecutor () noexcept
{
    terminating.test_and_set (std::memory_order_acquire);
    hasJobs.test_and_set (std::memory_order_release);
    hasJobs.notify_all ();

    for (std::jthread &thread : threads)
    {
        thread.join ();
    }
}

void JobExecutor::Push (JobExecutor::Job _task) noexcept
{
    while (true)
    {
        AtomicFlagGuard guard {modifyingPool};
        jobPool.emplace_back (std::move (_task));
        hasJobs.test_and_set (std::memory_order_release);
        hasJobs.notify_one ();
        return;
    }
}

JobExecutor::Job JobExecutor::Pop () noexcept
{
    while (true)
    {
        hasJobs.wait (false, std::memory_order_acquire);
        AtomicFlagGuard guard {modifyingPool};

        if (terminating.test (std::memory_order_acquire))
        {
            // Empty task to schedule termination.
            return [] ()
            {
            };
        }

        if (jobPool.empty ())
        {
            continue;
        }

        Job task {std::move (jobPool.back ())};
        jobPool.pop_back ();

        if (jobPool.empty ())
        {
            hasJobs.clear (std::memory_order_release);
        }

        return task;
    }
}

static JobExecutor globalJobExecutor {};

class ExecutorImplementation final
{
public:
    explicit ExecutorImplementation (const Collection &_collection) noexcept;

    ExecutorImplementation (const ExecutorImplementation &_other) = delete;

    ExecutorImplementation (ExecutorImplementation &&_other) = delete;

    ~ExecutorImplementation () noexcept = default;

    void Execute () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ExecutorImplementation);

private:
    struct Task final
    {
        std::function<void ()> executor;

        Container::Vector<std::size_t> dependantTasksIndices {Memory::Profiler::AllocationGroup::Top ()};

        std::size_t dependencyCount = 0u;

        /// \brief Count of unresolved dependencies for this task in current execution.
        /// \invariant After executor initialization and after every execution of this task
        ///            ::dependenciesLeftThisRun should be equal to ::dependencyCount.
        std::size_t dependenciesLeftThisRun = 0u;
    };

    void TaskFunction (std::size_t _taskIndex) noexcept;

    Container::Vector<Task> tasks;

    /// \details We cache entry tasks to make ::taskQueue initialization in ::Execute faster.
    Container::Vector<std::size_t> entryTaskIndices;

    /// \details Task modification is rare and chance that different threads are fighting for modification access
    ///          is low. Most performant mutual exclusion technique is such cases is atomic_flag-based spinlock.
    std::atomic_flag modifyingTasks;

    /// \brief Atomic flag for waiting until all tasks are executed.
    std::atomic_flag tasksExecuting;

    /// \brief Indicates how much unfinished tasks left during this execution.
    std::atomic_unsigned_lock_free tasksLeftToExecute = 0u;
};

using namespace Memory::Literals;

ExecutorImplementation::ExecutorImplementation (const Collection &_collection) noexcept
    : tasks (Memory::Profiler::AllocationGroup::Top ()),
      entryTaskIndices (Memory::Profiler::AllocationGroup::Top ())
{
    auto placeholder = tasks.get_allocator ().GetAllocationGroup ().PlaceOnTop ();
    tasks.resize (_collection.tasks.size ());

    for (std::size_t taskIndex = 0u; taskIndex < tasks.size (); ++taskIndex)
    {
        Task &task = tasks[taskIndex];
        const Collection::Item &item = _collection.tasks[taskIndex];

        task.executor = item.task;
        task.dependantTasksIndices = item.dependantTasksIndices;

        for (std::size_t dependantIndex : task.dependantTasksIndices)
        {
            ++tasks[dependantIndex].dependencyCount;
        }
    }

    for (std::size_t taskIndex = 0u; taskIndex < tasks.size (); ++taskIndex)
    {
        Task &task = tasks[taskIndex];
        task.dependenciesLeftThisRun = task.dependencyCount;

        if (task.dependencyCount == 0u)
        {
            entryTaskIndices.emplace_back (taskIndex);
        }
    }

    assert (!entryTaskIndices.empty ());
}

void ExecutorImplementation::Execute () noexcept
{
    assert (!entryTaskIndices.empty ());
    if (entryTaskIndices.empty ())
    {
        return;
    }

    tasksExecuting.test_and_set (std::memory_order_acquire);
    tasksLeftToExecute = tasks.size ();

    for (std::size_t taskIndex : entryTaskIndices)
    {
        globalJobExecutor.Push (
            [this, taskIndex] ()
            {
                TaskFunction (taskIndex);
            });
    }

    tasksExecuting.wait (true, std::memory_order_acquire);
}

void ExecutorImplementation::TaskFunction (std::size_t _taskIndex) noexcept
{
    Task &task = tasks[_taskIndex];
    assert (task.dependenciesLeftThisRun == 0u);
    task.executor ();
    task.dependenciesLeftThisRun = task.dependencyCount;

    LockAtomicFlag (modifyingTasks);
    for (std::size_t dependantIndex : task.dependantTasksIndices)
    {
        if (--tasks[dependantIndex].dependenciesLeftThisRun == 0u)
        {
            UnlockAtomicFlag (modifyingTasks);
            globalJobExecutor.Push (
                [this, dependantIndex] ()
                {
                    TaskFunction (dependantIndex);
                });

            LockAtomicFlag (modifyingTasks);
        }
    }

    UnlockAtomicFlag (modifyingTasks);
    if (--tasksLeftToExecute == 0u)
    {
        tasksExecuting.clear (std::memory_order_release);
        tasksExecuting.notify_one ();
    }
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup ("ParallelExecutor"_us)};
    ExecutorImplementation *executor = nullptr;
};

Executor::Executor (const Collection &_collection) noexcept
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();
    internal.executor = new (internal.heap.Acquire (sizeof (ExecutorImplementation), alignof (ExecutorImplementation)))
        ExecutorImplementation (_collection);
}

Executor::Executor (Executor &&_other) noexcept
{
    auto &internal = *new (&data) InternalData ();
    internal.executor = block_cast<InternalData> (_other.data).executor;
    block_cast<InternalData> (_other.data).executor = nullptr;
}

Executor::~Executor () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    if (internal.executor)
    {
        internal.executor->~ExecutorImplementation ();
        internal.heap.Release (internal.executor, sizeof (ExecutorImplementation));
    }

    internal.~InternalData ();
}

void Executor::Execute () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    assert (internal.executor);
    internal.executor->Execute ();
}
} // namespace Emergence::Task
