#include <cassert>
#include <stack>

#include <Task/Executor.hpp>

namespace Emergence::Task
{
class ExecutorImplementation
{
public:
    explicit ExecutorImplementation (const Collection &_collection) noexcept;

    ExecutorImplementation (const ExecutorImplementation &_other) = delete;

    ExecutorImplementation (ExecutorImplementation &&_other) = delete;

    ~ExecutorImplementation () = default;

    void Execute () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ExecutorImplementation);

private:
    std::vector<std::function<void ()>> orderedTasks;
};

ExecutorImplementation::ExecutorImplementation (const Collection &_collection) noexcept
{
    orderedTasks.reserve (_collection.tasks.size ());
    // We can not use {count, value} constructor here, because some compilers parse it as initializer list.
    std::vector<std::size_t> dependenciesLeft;
    dependenciesLeft.resize (_collection.tasks.size (), 0u);

    for (const Collection::Item &item : _collection.tasks)
    {
        for (std::size_t dependantIndex : item.dependantTasksIndices)
        {
            assert (dependantIndex < dependenciesLeft.size ());
            ++dependenciesLeft[dependantIndex];
        }
    }

    std::stack<std::size_t> resolvedTasks;
    for (std::size_t index = 0u; index < dependenciesLeft.size (); ++index)
    {
        if (dependenciesLeft[index] == 0u)
        {
            resolvedTasks.push (index);
        }
    }

    std::size_t resolvedTasksCount = 0u;
    while (!resolvedTasks.empty ())
    {
        ++resolvedTasksCount;
        std::size_t taskIndex = resolvedTasks.top ();
        resolvedTasks.pop ();

        assert (taskIndex < _collection.tasks.size ());
        orderedTasks.emplace_back (_collection.tasks[taskIndex].task);

        for (std::size_t dependantIndex : _collection.tasks[taskIndex].dependantTasksIndices)
        {
            if (--dependenciesLeft[dependantIndex] == 0u)
            {
                resolvedTasks.push (dependantIndex);
            }
        }
    }

    // This assert fails if task collection contains circular dependencies.
    assert (resolvedTasksCount == _collection.tasks.size ());
}

void ExecutorImplementation::Execute () const noexcept
{
    for (const auto &task : orderedTasks)
    {
        task ();
    }
}

Executor::Executor (const Collection &_collection, std::size_t /*unused*/) noexcept
    : handle (new ExecutorImplementation (_collection))
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
