#include <atomic>
#include <barrier>
#include <cassert>
#include <thread>

#include <Container/Vector.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>
#include <SyntaxSugar/BlockCast.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Task
{
class ExecutorImplementation final
{
public:
    explicit ExecutorImplementation (const Collection &_collection, std::size_t _workers) noexcept;

    ExecutorImplementation (const ExecutorImplementation &_other) = delete;

    ExecutorImplementation (ExecutorImplementation &&_other) = delete;

    ~ExecutorImplementation () noexcept;

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

    void WorkerMain () noexcept;

    Container::Vector<Task> tasks;

    /// \details We cache entry tasks to make ::taskQueue initialization in ::Execute faster.
    Container::Vector<std::size_t> entryTaskIndices;

    Container::Vector<std::jthread> workers;

    /// \details Task queue modification is rare and chance that different threads are fighting for modification access
    ///          is low. Most performant mutual exclusion technique is such cases is atomic_flag-based spinlock.
    std::atomic_flag modifyingTaskQueue;

    /// \details Used to pause workers, that are waiting for tasks, when there is no sense to check task queue.
    std::atomic_flag taskQueueNotEmptyOrAllTasksFinished;

    std::size_t tasksFinished = 0u;
    std::size_t tasksInExecution = 0u;
    Container::Vector<std::size_t> tasksQueue;

    /// \details Workers are paused at this barrier in two cases:
    ///          - They are waiting for new execution routine.
    ///          - They finished executing ::WorkerMain during current routine. Waiting here is used
    ///            to ensure that all workers will be stopped when main thread exits from ::Execute.
    std::barrier<> workerExecutionBarrier;

    /// \details Informs workers that they should exit. Used only in destructor.
    std::atomic_flag terminating;
};

using namespace Memory::Literals;

ExecutorImplementation::ExecutorImplementation (const Collection &_collection, std::size_t _workers) noexcept
    : tasks (Memory::Profiler::AllocationGroup::Top ()),
      entryTaskIndices (Memory::Profiler::AllocationGroup::Top ()),
      workers (Memory::Profiler::AllocationGroup::Top ()),
      tasksQueue (Memory::Profiler::AllocationGroup::Top ()),
      workerExecutionBarrier (static_cast<ptrdiff_t> (_workers + 1u))
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
    workers.reserve (_workers);

    for (std::size_t workerIndex = 0u; workerIndex < _workers; ++workerIndex)
    {
        workers.emplace_back (
            [this] ()
            {
                while (true)
                {
                    workerExecutionBarrier.arrive_and_wait ();
                    if (terminating.test (std::memory_order_acquire))
                    {
                        return;
                    }

                    WorkerMain ();
                    workerExecutionBarrier.arrive_and_wait ();
                }
            });
    }
}

ExecutorImplementation::~ExecutorImplementation () noexcept
{
    // Set termination flag and awake all workers.
    terminating.test_and_set (std::memory_order_acquire);
    workerExecutionBarrier.arrive_and_wait ();

    for (std::jthread &worker : workers)
    {
        worker.join ();
    }
}

void ExecutorImplementation::Execute () noexcept
{
    assert (!entryTaskIndices.empty ());
    if (entryTaskIndices.empty ())
    {
        return;
    }

    // Ensure that synchronization variable is in clean state.
    assert (!modifyingTaskQueue.test ());

    tasksQueue = entryTaskIndices;
    taskQueueNotEmptyOrAllTasksFinished.test_and_set (std::memory_order_acquire);

    // Awake workers and switch main thread to worker mode.
    workerExecutionBarrier.arrive_and_wait ();
    WorkerMain ();
    workerExecutionBarrier.arrive_and_wait ();

    // Clear shared state after execution.
    tasksFinished = 0u;
    tasksInExecution = 0u;
}

void ExecutorImplementation::WorkerMain () noexcept
{
    while (true)
    {
        Task *currentTask = nullptr;

        // Extracting next available task from queue.
        while (!currentTask)
        {
            taskQueueNotEmptyOrAllTasksFinished.wait (false, std::memory_order_acquire);
            AtomicFlagGuard guard {modifyingTaskQueue};

            bool exit = false;
            if (!tasksQueue.empty ())
            {
                assert (tasksFinished != tasks.size ());
                currentTask = &tasks[tasksQueue.back ()];
                tasksQueue.pop_back ();
                ++tasksInExecution;
            }
            else
            {
                const bool allTasksFinished = tasksFinished == tasks.size ();

                // There is no tasks in execution and not all tasks are finished,
                // but task queue is empty -- looks like a classic deadlock.
                const bool deadlockDetected = !allTasksFinished && tasksInExecution == 0u;

                assert (!deadlockDetected);
                exit = allTasksFinished || deadlockDetected;
            }

            // If there is no tasks and if we are not exiting, we should inform
            // other workers that there is no sense to check task queue right now.
            if (tasksQueue.empty () && !exit)
            {
                taskQueueNotEmptyOrAllTasksFinished.clear (std::memory_order_release);
            }
            
            if (exit)
            {
                return;
            }
        }

        assert (currentTask->dependenciesLeftThisRun == 0u);
        currentTask->executor ();
        currentTask->dependenciesLeftThisRun = currentTask->dependencyCount;

        // Registering successful task execution and unlocking dependant tasks.
        AtomicFlagGuard guard {modifyingTaskQueue};

        ++tasksFinished;
        --tasksInExecution;

        const bool taskQueueWasEmpty = tasksQueue.empty ();
        for (std::size_t dependantIndex : currentTask->dependantTasksIndices)
        {
            if (--tasks[dependantIndex].dependenciesLeftThisRun == 0u)
            {
                tasksQueue.emplace_back (dependantIndex);
            }
        }

        // If we added tasks to empty task queue or if all tasks are finished,
        // we should resume other workers to make them able to grab tasks or exit.
        if ((taskQueueWasEmpty && !tasksQueue.empty ()) || tasksFinished == tasks.size ())
        {
            taskQueueNotEmptyOrAllTasksFinished.test_and_set (std::memory_order_seq_cst);
            taskQueueNotEmptyOrAllTasksFinished.notify_all ();
        }
    }
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup ("ParallelExecutor"_us)};
    ExecutorImplementation *executor = nullptr;
};

Executor::Executor (const Collection &_collection, std::size_t _maximumChildThreads) noexcept
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();
    internal.executor = new (internal.heap.Acquire (sizeof (ExecutorImplementation)))
        ExecutorImplementation (_collection, _maximumChildThreads);
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
