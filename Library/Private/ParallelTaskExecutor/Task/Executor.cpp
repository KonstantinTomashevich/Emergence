#include <atomic>
#include <cassert>
#include <thread>

#include <Container/Vector.hpp>

#include <Job/Dispatcher.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>
#include <SyntaxSugar/BlockCast.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Task
{
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
        Job::Dispatcher::Global ().Dispatch (
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
            Job::Dispatcher::Global ().Dispatch (
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
