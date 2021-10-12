#include <atomic>
#include <cassert>
#include <thread>
#include <vector>

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

        std::vector<std::size_t> dependantTasksIndices;

        std::size_t dependencyCount = 0u;

        std::size_t dependenciesLeftThisRun = 0u;
    };

    void WorkerMain () noexcept;

    std::vector<Task> tasks;
    std::vector<std::size_t> entryTaskIndices;
    std::vector<std::jthread> workers;

    std::atomic_flag modifyingTaskQueue;
    std::atomic_flag taskQueueNotEmptyOrAllTasksFinished;

    std::size_t tasksFinished = 0u;
    std::size_t tasksInExecution = 0u;
    std::vector<std::size_t> tasksQueue;

    std::atomic_flag workersAwake;
    bool terminating = false;
};

ExecutorImplementation::ExecutorImplementation (const Collection &_collection, std::size_t _workers) noexcept
{
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
                    workersAwake.wait (false);
                    if (terminating)
                    {
                        return;
                    }

                    WorkerMain ();
                }
            });
    }
}

ExecutorImplementation::~ExecutorImplementation () noexcept
{
    terminating = true;
    workersAwake.test_and_set ();
    workersAwake.notify_all ();

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

    assert (!workersAwake.test ());
    assert (!modifyingTaskQueue.test ());

    tasksQueue = entryTaskIndices;
    taskQueueNotEmptyOrAllTasksFinished.test_and_set ();

    tasksFinished = 0u;
    tasksInExecution = 0u;

    workersAwake.test_and_set ();
    workersAwake.notify_all ();

    WorkerMain ();
    workersAwake.clear ();

    // TODO: Somehow wait till all workers finish executing WorkerMain.
}

void ExecutorImplementation::WorkerMain () noexcept
{
    while (true)
    {
        // TODO: Think about memory order.

        Task *currentTask = nullptr;
        while (!currentTask)
        {
            taskQueueNotEmptyOrAllTasksFinished.wait (false);

            while (modifyingTaskQueue.test_and_set ())
            {
                std::this_thread::yield ();
            }

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
                const bool deadlockDetected = !allTasksFinished && tasksInExecution == 0u;

                assert (!deadlockDetected);
                exit = allTasksFinished || deadlockDetected;
            }

            if (tasksQueue.empty () && !exit)
            {
                taskQueueNotEmptyOrAllTasksFinished.clear ();
            }

            modifyingTaskQueue.clear ();

            if (exit)
            {
                return;
            }
        }

        assert (currentTask->dependenciesLeftThisRun == 0u);
        currentTask->executor ();
        currentTask->dependenciesLeftThisRun = currentTask->dependencyCount;

        while (modifyingTaskQueue.test_and_set ())
        {
            std::this_thread::yield ();
        }

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

        if ((taskQueueWasEmpty && !tasksQueue.empty ()) || tasksFinished == tasks.size ())
        {
            taskQueueNotEmptyOrAllTasksFinished.test_and_set ();
            taskQueueNotEmptyOrAllTasksFinished.notify_all ();
        }

        modifyingTaskQueue.clear ();
    }
}

Executor::Executor (const Collection &_collection, std::size_t _maximumChildThreads) noexcept
    : handle (new ExecutorImplementation (_collection, _maximumChildThreads))
{
}

Executor::Executor (Executor &&_other) noexcept : handle (_other.handle)
{
    _other.handle = nullptr;
}

Executor::~Executor () noexcept
{
    delete static_cast<ExecutorImplementation *> (handle);
}

void Executor::Execute () noexcept
{
    assert (handle);
    static_cast<ExecutorImplementation *> (handle)->Execute ();
}
} // namespace Emergence::Task
