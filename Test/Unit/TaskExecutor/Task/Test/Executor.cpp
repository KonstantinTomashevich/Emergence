#include <chrono>
#include <thread>

#include <Container/Algorithm.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>
#include <Memory/UniqueString.hpp>

#include <Task/Executor.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Task::Test
{
struct TaskSeed
{
    Memory::UniqueString name;
    Container::Vector<Memory::UniqueString> dependantTasks;
};

using Seed = Container::Vector<TaskSeed>;

struct TimeInterval
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
};

void GrowAndTest (const Seed &_seed)
{
    Container::Vector<TimeInterval> intervals;
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
            std::this_thread::sleep_for (1us);
            privateInterval.end = std::chrono::high_resolution_clock::now ();
        };

        for (const Memory::UniqueString &dependantTaskName : taskSeed.dependantTasks)
        {
            auto iterator = Container::FindIf (_seed.begin (), _seed.end (),
                                               [&dependantTaskName] (const TaskSeed &_other)
                                               {
                                                   return _other.name == dependantTaskName;
                                               });

            REQUIRE (iterator != _seed.end ());
            taskItem.dependantTasksIndices.emplace_back (std::distance (_seed.begin (), iterator));
        }
    }

    Executor executor {collection};
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

using namespace Emergence::Memory::Literals;
using namespace Emergence::Task::Test;

BEGIN_SUITE (Executor)

TEST_CASE (Sequence)
{
    GrowAndTest ({{"A"_us, {"B"_us}}, {"B"_us, {"C"_us}}, {"C"_us, {}}});
}

TEST_CASE (TwoIndependentSequence)
{
    GrowAndTest ({{"A1"_us, {"B1"_us}},
                  {"B1"_us, {"C1"_us}},
                  {"C1"_us, {}},
                  {"A2"_us, {"B2"_us}},
                  {"B2"_us, {"C2"_us}},
                  {"C2"_us, {}}});
}

TEST_CASE (ForkJoin)
{
    GrowAndTest ({{"A"_us, {"B1"_us, "B2"_us, "B3"_us, "B4"_us}},
                  {"B1"_us, {"C1"_us}},
                  {"B2"_us, {"C2"_us}},
                  {"B3"_us, {"C3"_us}},
                  {"B4"_us, {"C4"_us}},
                  {"C1"_us, {"D"_us}},
                  {"C2"_us, {"D"_us}},
                  {"C3"_us, {"D"_us}},
                  {"C4"_us, {"D"_us}},
                  {"D"_us, {}}});
}

TEST_CASE (MultipleEntries)
{
    GrowAndTest ({{"A1"_us, {"B1"_us}},
                  {"A2"_us, {"B1"_us, "B2"_us}},
                  {"A3"_us, {"B2"_us, "B3"_us}},
                  {"B1"_us, {"C"_us}},
                  {"B2"_us, {"C"_us}},
                  {"B3"_us, {"D"_us}},
                  {"C"_us, {}},
                  {"D"_us, {}}});
}

TEST_CASE (ReversedRegistrationOrder)
{
    GrowAndTest ({{"C"_us, {}}, {"B"_us, {"C"_us}}, {"A"_us, {"B"_us}}});
}

END_SUITE
