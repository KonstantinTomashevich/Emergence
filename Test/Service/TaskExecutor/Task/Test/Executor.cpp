#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <Task/Executor.hpp>
#include <Task/Test/Executor.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Task::Test
{
bool ExecutorTestIncludeMarker () noexcept
{
    return true;
}

struct TaskSeed
{
    std::string name;
    std::vector<std::string> dependantTasks;
};

using Seed = std::vector<TaskSeed>;

struct TimeInterval
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
};

void GrowAndTest (const Seed &_seed)
{
    std::vector<TimeInterval> intervals;
    intervals.reserve (_seed.size ());
    Collection collection;

    for (const TaskSeed &taskSeed : _seed)
    {
        TimeInterval &privateInterval = intervals.emplace_back ();
        Collection::Item &taskItem = collection.tasks.emplace_back ();
        taskItem.name = taskSeed.name;

        taskItem.task = [&privateInterval] ()
        {
            using namespace std::chrono_literals;
            privateInterval.start = std::chrono::high_resolution_clock::now ();
            std::this_thread::sleep_for (10ns);
            privateInterval.end = std::chrono::high_resolution_clock::now ();
        };

        for (const std::string &dependantTaskName : taskSeed.dependantTasks)
        {
            auto iterator = std::find_if (_seed.begin (), _seed.end (),
                                          [&dependantTaskName] (const TaskSeed &_other)
                                          {
                                              return _other.name == dependantTaskName;
                                          });

            REQUIRE (iterator != _seed.end ());
            taskItem.dependantTasksIndices.emplace_back (std::distance (_seed.begin (), iterator));
        }
    }

    Executor executor {collection, 4u};
    auto executeAndTest = [&executor, &intervals, &collection] ()
    {
        executor.Execute ();
        for (std::size_t sourceIndex = 0u; sourceIndex < collection.tasks.size (); ++sourceIndex)
        {
            const Collection::Item &source = collection.tasks[sourceIndex];
            LOG ("Checking task ", source.name, " and its targets.");
            const TimeInterval &sourceInterval = intervals[sourceIndex];

            // We use sleep to artificially make tasks longer, therefore start and end time can not be equal.
            CHECK (sourceInterval.start < sourceInterval.end);

            for (std::size_t targetIndex : source.dependantTasksIndices)
            {
                const Collection::Item &target = collection.tasks[targetIndex];
                LOG ("Checking target ", target.name, ".");
                const TimeInterval &targetInterval = intervals[targetIndex];
                CHECK (sourceInterval.end <= targetInterval.start);
            }
        }
    };

    // Execute and test twice to make sure that executors are reusable.
    executeAndTest ();
    executeAndTest ();
}
} // namespace Emergence::Task::Test

using namespace Emergence::Task::Test;

BEGIN_SUITE (Executor)

TEST_CASE (Sequence)
{
    GrowAndTest ({{"A", {"B"}}, {"B", {"C"}}, {"C", {}}});
}

TEST_CASE (TwoIndependentSequence)
{
    GrowAndTest ({{"A1", {"B1"}}, {"B1", {"C1"}}, {"C1", {}}, {"A2", {"B2"}}, {"B2", {"C2"}}, {"C2", {}}});
}

TEST_CASE (ForkJoin)
{
    GrowAndTest ({{"A", {"B1", "B2", "B3", "B4"}},
                  {"B1", {"C1"}},
                  {"B2", {"C2"}},
                  {"B3", {"C3"}},
                  {"B4", {"C4"}},
                  {"C1", {"D"}},
                  {"C2", {"D"}},
                  {"C3", {"D"}},
                  {"C4", {"D"}},
                  {"D", {}}});
}

TEST_CASE (MultipleEntries)
{
    GrowAndTest ({{"A1", {"B1"}},
                  {"A2", {"B1", "B2"}},
                  {"A3", {"B2", "B3"}},
                  {"B1", {"C"}},
                  {"B2", {"C"}},
                  {"B3", {"D"}},
                  {"C", {}},
                  {"D", {}}});
}

TEST_CASE (ReversedRegistrationOrder)
{
    GrowAndTest ({{"C", {}}, {"B", {"C"}}, {"A", {"B"}}});
}

END_SUITE
