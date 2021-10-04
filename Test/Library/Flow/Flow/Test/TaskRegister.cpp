#include <string>
#include <vector>

#include <Flow/TaskRegister.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Flow::Test
{
struct Seed
{
    std::vector<Task> tasks;
    std::vector<std::string> checkpoints;
    std::vector<std::string> resources;
};

TaskCollection GrowCollection (const Seed &_seed)
{
    TaskRegister taskRegister;
    for (const Task &task : _seed.tasks)
    {
        taskRegister.RegisterTask (task);
    }

    for (const std::string &checkpoint : _seed.checkpoints)
    {
        taskRegister.RegisterCheckpoint (checkpoint.c_str ());
    }

    for (const std::string &resource : _seed.resources)
    {
        taskRegister.RegisterResource (resource.c_str ());
    }

    return taskRegister.ExportCollection ();
}

struct CollectionExpectation final
{
    struct Task
    {
        std::string name;
        std::vector<std::string> dependencies;
    };

    std::vector<Task> tasks;
};

static void Check (const TaskCollection &_result, const CollectionExpectation &_expectation)
{
    CHECK_EQUAL (_result.tasks.size (), _expectation.tasks.size ());
    const std::size_t tasksToCheck = std::min (_result.tasks.size (), _expectation.tasks.size ());

    for (std::size_t taskIndex = 0u; taskIndex < tasksToCheck; ++taskIndex)
    {
        LOG ("Checking task at index ", taskIndex, ".");
        const auto &result = _result.tasks[taskIndex];
        const auto &expectation = _expectation.tasks[taskIndex];

        CHECK_EQUAL (result.name, expectation.name);
        // Unfortunately, we can not check task executors equality, because it's not supported by std::function.

        CHECK_EQUAL (result.dependencyIndices.size (), expectation.dependencies.size ());
        const std::size_t dependenciesToCheck =
            std::min (result.dependencyIndices.size (), expectation.dependencies.size ());

        for (std::size_t dependencyIndex = 0u; dependencyIndex < dependenciesToCheck; ++dependencyIndex)
        {
            LOG ("Checking dependency at index ", dependencyIndex, ".");
            std::size_t resultIndex = result.dependencyIndices[dependencyIndex];

            if (resultIndex < _expectation.tasks.size ())
            {
                CHECK_EQUAL (_expectation.tasks[resultIndex].name, expectation.dependencies[dependencyIndex]);
            }
            else
            {
                CHECK_WITH_MESSAGE (false, "Result index ", resultIndex, " is too big!");
            }
        }
    }
}
} // namespace Emergence::Flow::Test

BEGIN_SUITE (ExportVisual)

// TODO: Tests.

END_SUITE

BEGIN_SUITE (ExportCollection)

TEST_CASE (Sequence)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection (
               {{{"A", nullptr, {}, {}, {}, {"B"}}, {"B", nullptr, {}, {}, {}, {}}, {"C", nullptr, {}, {}, {"B"}, {}}},
                {},
                {}}),
           {{{"A", {}}, {"B", {"A"}}, {"C", {"B"}}}});
}

TEST_CASE (SequenceWithCheckpoint)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{{"A", nullptr, {}, {}, {}, {"B"}}, {"C", nullptr, {}, {}, {"B"}, {}}}, {"B"}, {}}),
           {{{"A", {}}, {"C", {"A"}}}});
}

TEST_CASE (CheckpointAsBarrier)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A1", nullptr, {}, {}, {}, {"Join"}},
                                {"A2", nullptr, {}, {}, {}, {"Join"}},
                                {"B1", nullptr, {}, {}, {"Join"}, {}},
                                {"B2", nullptr, {}, {}, {"Join"}, {}},
                            },
                            {"Join"},
                            {}}),
           {{{"A1", {}}, {"A2", {}}, {"B1", {"A1", "A2"}}, {"B2", {"A1", "A2"}}}});
}

TEST_CASE (ComplexDependencies)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A1", nullptr, {}, {}, {}, {"Join1"}},
                                {"A2", nullptr, {}, {}, {}, {"Join1"}},
                                {"A3", nullptr, {}, {}, {}, {"Join2"}},
                                {"A4", nullptr, {}, {}, {}, {"Join2"}},
                                {"A5", nullptr, {}, {}, {}, {}},
                                {"B1", nullptr, {}, {}, {"Join1", "Join2"}, {}},
                                {"B2", nullptr, {}, {}, {"Join1", "A5", "B1"}, {}},
                            },
                            {"Join1", "Join2"},
                            {}}),
           {{{"A1", {}},
             {"A2", {}},
             {"A3", {}},
             {"A4", {}},
             {"A5", {}},
             {"B1", {"A1", "A2", "A3", "A4"}},
             {"B2", {"A1", "A2", "A5", "B1"}}}});
}

TEST_CASE (TrivialCycle)
{
    using namespace Emergence::Flow::Test;
    Check (
        GrowCollection (
            {{{"A", nullptr, {}, {}, {}, {"B"}}, {"B", nullptr, {}, {}, {}, {"C"}}, {"C", nullptr, {}, {}, {}, {"A"}}},
             {},
             {}}),
        {});
}

TEST_CASE (ComplexCycle)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A1", nullptr, {}, {}, {"B2"}, {"Join1"}},
                                {"A2", nullptr, {}, {}, {}, {"Join1"}},
                                {"A3", nullptr, {}, {}, {}, {"Join2"}},
                                {"A4", nullptr, {}, {}, {}, {"Join2"}},
                                {"A5", nullptr, {}, {}, {}, {}},
                                {"B1", nullptr, {}, {}, {"Join1", "Join2"}, {}},
                                {"B2", nullptr, {}, {}, {"Join1", "A5", "B1"}, {}},
                            },
                            {"Join1", "Join2"},
                            {}}),
           {});
}

TEST_CASE (DependencyNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{{"A", nullptr, {}, {}, {}, {"D"}}, {"B", nullptr, {}, {}, {"Join2"}, {}}}, {"Join1"}, {}}),
           {});
}

TEST_CASE (DuplicateResourceUsageInAccessMask)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{{"A", nullptr, {"R1"}, {"R1"}, {}, {}}}, {}, {"R1"}}), {});
}

TEST_CASE (TrivialSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A1", nullptr, {"R1"}, {"R2"}, {}, {"Join"}},
                                {"A2", nullptr, {"R1"}, {"R3"}, {}, {"Join"}},
                                {"B1", nullptr, {"R2"}, {"R1"}, {"Join"}, {}},
                                {"B2", nullptr, {}, {"R3"}, {"Join"}, {}},
                            },
                            {"Join"},
                            {"R1", "R2", "R3"}}),
           {{{"A1", {}}, {"A2", {}}, {"B1", {"A1", "A2"}}, {"B2", {"A1", "A2"}}}});
}

TEST_CASE (ChainedSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                                {"B1", nullptr, {"R2"}, {"R3"}, {"Join"}, {"C1"}},
                                {"C1", nullptr, {"R3"}, {}, {"Join"}, {}},
                                {"B2", nullptr, {"R2"}, {"R1"}, {"Join"}, {"C2"}},
                                {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                            },
                            {"Join"},
                            {"R1", "R2", "R3"}}),
           {{{"A", {}}, {"B1", {"A"}}, {"C1", {"B1"}}, {"B2", {"A"}}, {"C2", {"B2"}}}});
}

TEST_CASE (ReadWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                                {"B1", nullptr, {"R2"}, {"R3"}, {"Join"}, {"C1"}},
                                {"C1", nullptr, {"R3"}, {"R2"}, {"Join"}, {}},
                                {"B2", nullptr, {"R2"}, {"R1"}, {"Join"}, {"C2"}},
                                {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                            },
                            {"Join"},
                            {"R1", "R2", "R3"}}),
           {});
}

TEST_CASE (WriteWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{
                                {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                                {"B1", nullptr, {}, {"R3"}, {"Join"}, {"C1"}},
                                {"C1", nullptr, {"R3"}, {"R2"}, {"Join"}, {}},
                                {"B2", nullptr, {}, {"R2"}, {"Join"}, {"C2"}},
                                {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                            },
                            {"Join"},
                            {"R1", "R2", "R3"}}),
           {});
}

TEST_CASE (ResourceNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (GrowCollection ({{{"A", nullptr, {}, {"R1"}, {}, {}}, {"B", nullptr, {}, {"R2"}, {}, {}}}, {}, {}}), {});
}

END_SUITE
