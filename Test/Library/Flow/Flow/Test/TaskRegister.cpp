#include <Container/String.hpp>

#include <Flow/TaskRegister.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;

namespace Emergence::Flow::Test
{
struct Seed final
{
    Container::Vector<Task> tasks;
    Container::Vector<Memory::UniqueString> checkpoints;
    Container::Vector<Memory::UniqueString> resources;
};

void Grow (const Seed &_seed, TaskRegister &_reusableRegister)
{
    for (const Task &task : _seed.tasks)
    {
        _reusableRegister.RegisterTask (task);
    }

    for (Memory::UniqueString checkpoint : _seed.checkpoints)
    {
        _reusableRegister.RegisterCheckpoint (checkpoint);
    }

    for (Memory::UniqueString resource : _seed.resources)
    {
        _reusableRegister.RegisterResource (resource);
    }
}

TaskRegister Grow (const Seed &_seed)
{
    TaskRegister taskRegister;
    Grow (_seed, taskRegister);
    return taskRegister;
}

struct CollectionExpectation final
{
    struct Task
    {
        Container::String name;
        Container::Vector<Container::String> dependantTasks;
    };

    Container::Vector<Task> tasks;
};

static void Check (const Emergence::Task::Collection &_result, const CollectionExpectation &_expectation)
{
    CHECK_EQUAL (_result.tasks.size (), _expectation.tasks.size ());
    const std::size_t tasksToCheck = std::min (_result.tasks.size (), _expectation.tasks.size ());

    for (std::size_t taskIndex = 0u; taskIndex < tasksToCheck; ++taskIndex)
    {
        LOG ("Checking task at index ", taskIndex, ".");
        const auto &result = _result.tasks[taskIndex];
        const auto &expectation = _expectation.tasks[taskIndex];

        CHECK_EQUAL (*result.name, expectation.name);
        // Unfortunately, we can not check task executors equality, because it's not supported by std::function.

        CHECK_EQUAL (result.dependantTasksIndices.size (), expectation.dependantTasks.size ());
        const std::size_t dependantTasksToCheck =
            std::min (result.dependantTasksIndices.size (), expectation.dependantTasks.size ());

        for (std::size_t dependantTaskIndex = 0u; dependantTaskIndex < dependantTasksToCheck; ++dependantTaskIndex)
        {
            LOG ("Checking dependant task at index ", dependantTaskIndex, ".");
            std::size_t resultIndex = result.dependantTasksIndices[dependantTaskIndex];
            CHECK_EQUAL (_expectation.tasks[resultIndex].name, expectation.dependantTasks[dependantTaskIndex]);
        }
    }
}

static Container::String CheckpointNodeLabel (Container::String _name)
{
    return _name += TaskRegister::VISUAL_CHECKPOINT_LABEL_SUFFIX;
}

static Container::String TaskNodeLabel (Container::String _name)
{
    return _name += TaskRegister::VISUAL_TASK_LABEL_SUFFIX;
}

static Container::String ResourceNodePath (const char *_name)
{
    return TaskRegister::VISUAL_RESOURCE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _name;
}

static Container::String TaskNodePath (const char *_name)
{
    return TaskRegister::VISUAL_PIPELINE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _name;
}

static const Seed TRIVIAL_SAFE_RESOURCE_USAGE_SEED {{
                                                        {"A1"_us, nullptr, {"R1"_us}, {"R2"_us}, {}, {"Join"_us}},
                                                        {"A2"_us, nullptr, {"R1"_us}, {"R3"_us}, {}, {"Join"_us}},
                                                        {"B1"_us, nullptr, {"R2"_us}, {"R1"_us}, {"Join"_us}, {}},
                                                        {"B2"_us, nullptr, {}, {"R3"_us}, {"Join"_us}, {}},
                                                    },
                                                    {"Join"_us},
                                                    {"R1"_us, "R2"_us, "R3"_us}};

VisualGraph::Graph GetTrivialSafeResourceUsageGraph ()
{
    return {
        TaskRegister::VISUAL_ROOT_GRAPH_ID,
        {},
        {{TaskRegister::VISUAL_RESOURCE_GRAPH_ID, {}, {}, {{"R1", {}}, {"R2", {}}, {"R3", {}}}, {}},
         {TaskRegister::VISUAL_PIPELINE_GRAPH_ID,
          {},
          {},
          {{"Join", CheckpointNodeLabel ("Join")},
           {"A1", TaskNodeLabel ("A1")},
           {"A2", TaskNodeLabel ("A2")},
           {"B1", TaskNodeLabel ("B1")},
           {"B2", TaskNodeLabel ("B2")}},
          {{"A1", "Join", {}}, {"A2", "Join", {}}, {"Join", "B1", {}}, {"Join", "B2", {}}}}},
        {},
        {{TaskNodePath ("A1"), ResourceNodePath ("R1"), TaskRegister::VISUAL_READ_ACCESS_COLOR},
         {TaskNodePath ("A1"), ResourceNodePath ("R2"), TaskRegister::VISUAL_WRITE_ACCESS_COLOR},
         {TaskNodePath ("A2"), ResourceNodePath ("R1"), TaskRegister::VISUAL_READ_ACCESS_COLOR},
         {TaskNodePath ("A2"), ResourceNodePath ("R3"), TaskRegister::VISUAL_WRITE_ACCESS_COLOR},
         {TaskNodePath ("B1"), ResourceNodePath ("R2"), TaskRegister::VISUAL_READ_ACCESS_COLOR},
         {TaskNodePath ("B1"), ResourceNodePath ("R1"), TaskRegister::VISUAL_WRITE_ACCESS_COLOR},
         {TaskNodePath ("B2"), ResourceNodePath ("R3"), TaskRegister::VISUAL_WRITE_ACCESS_COLOR}},
    };
}

CollectionExpectation GetTrivialSafeResourceUsageExpectation ()
{
    return {{{"A1", {"B1", "B2"}}, {"A2", {"B1", "B2"}}, {"B1", {}}, {"B2", {}}}};
}

static const Seed COMPLEX_DEPENDENCIES_SEED {{
                                                 {"A1"_us, nullptr, {}, {}, {}, {"Join1"_us}},
                                                 {"A2"_us, nullptr, {}, {}, {}, {"Join1"_us}},
                                                 {"A3"_us, nullptr, {}, {}, {}, {"Join2"_us}},
                                                 {"A4"_us, nullptr, {}, {}, {}, {"Join2"_us}},
                                                 {"A5"_us, nullptr, {}, {}, {}, {}},
                                                 {"B1"_us, nullptr, {}, {}, {"Join1"_us, "Join2"_us}, {}},
                                                 {"B2"_us, nullptr, {}, {}, {"Join1"_us, "A5"_us, "B1"_us}, {}},
                                             },
                                             {"Join1"_us, "Join2"_us},
                                             {}};

CollectionExpectation GetComplexDependenciesExpectation ()
{
    return {{{"A1", {"B1", "B2"}},
             {"A2", {"B1", "B2"}},
             {"A3", {"B1"}},
             {"A4", {"B1"}},
             {"A5", {"B2"}},
             {"B1", {"B2"}},
             {"B2", {}}}};
}
} // namespace Emergence::Flow::Test

BEGIN_SUITE (ExportCollection)

TEST_CASE (Sequence)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A"_us, nullptr, {}, {}, {}, {"B"_us}},
                   {"B"_us, nullptr, {}, {}, {}, {}},
                   {"C"_us, nullptr, {}, {}, {"B"_us}, {}}},
                  {},
                  {}})
               .ExportCollection (),
           {{{"A", {"B"}}, {"B", {"C"}}, {"C", {}}}});
}

TEST_CASE (SequenceWithCheckpoint)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A"_us, nullptr, {}, {}, {}, {"B"_us}}, {"C"_us, nullptr, {}, {}, {"B"_us}, {}}}, {"B"_us}, {}})
               .ExportCollection (),
           {{{"A", {"C"}}, {"C", {}}}});
}

TEST_CASE (CheckpointAsBarrier)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A1"_us, nullptr, {}, {}, {}, {"Join"_us}},
                      {"A2"_us, nullptr, {}, {}, {}, {"Join"_us}},
                      {"B1"_us, nullptr, {}, {}, {"Join"_us}, {}},
                      {"B2"_us, nullptr, {}, {}, {"Join"_us}, {}},
                  },
                  {"Join"_us},
                  {}})
               .ExportCollection (),
           {{{"A1", {"B1", "B2"}}, {"A2", {"B1", "B2"}}, {"B1", {}}, {"B2", {}}}});
}

TEST_CASE (ComplexDependencies)
{
    using namespace Emergence::Flow::Test;
    Check (Grow (COMPLEX_DEPENDENCIES_SEED).ExportCollection (), GetComplexDependenciesExpectation ());
}

TEST_CASE (TrivialCircularDependency)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A"_us, nullptr, {}, {}, {}, {"B"_us}},
                   {"B"_us, nullptr, {}, {}, {}, {"C"_us}},
                   {"C"_us, nullptr, {}, {}, {}, {"A"_us}}},
                  {},
                  {}})
               .ExportCollection (),
           {});
}

TEST_CASE (ComplexCircularDependency)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A1"_us, nullptr, {}, {}, {"B2"_us}, {"Join1"_us}},
                      {"A2"_us, nullptr, {}, {}, {}, {"Join1"_us}},
                      {"A3"_us, nullptr, {}, {}, {}, {"Join2"_us}},
                      {"A4"_us, nullptr, {}, {}, {}, {"Join2"_us}},
                      {"A5"_us, nullptr, {}, {}, {}, {}},
                      {"B1"_us, nullptr, {}, {}, {"Join1"_us, "Join2"_us}, {}},
                      {"B2"_us, nullptr, {}, {}, {"Join1"_us, "A5"_us, "B1"_us}, {}},
                  },
                  {"Join1"_us, "Join2"_us},
                  {}})
               .ExportCollection (),
           {});
}

TEST_CASE (DependencyNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (
        Grow (
            {{{"A"_us, nullptr, {}, {}, {}, {"D"_us}}, {"B"_us, nullptr, {}, {}, {"Join2"_us}, {}}}, {"Join1"_us}, {}})
            .ExportCollection (),
        {});
}

TEST_CASE (DuplicateResourceUsageInAccessMask)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A"_us, nullptr, {"R1"_us}, {"R1"_us}, {}, {}}}, {}, {"R1"_us}}).ExportCollection (), {});
}

TEST_CASE (TrivialSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (Grow (TRIVIAL_SAFE_RESOURCE_USAGE_SEED).ExportCollection (), GetTrivialSafeResourceUsageExpectation ());
}

TEST_CASE (ChainedSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A"_us, nullptr, {"R1"_us}, {"R2"_us}, {}, {"B1"_us, "B2"_us}},
                      {"B1"_us, nullptr, {"R2"_us}, {"R3"_us}, {"Join"_us}, {"C1"_us}},
                      {"C1"_us, nullptr, {"R3"_us}, {}, {"Join"_us}, {}},
                      {"B2"_us, nullptr, {"R2"_us}, {"R1"_us}, {"Join"_us}, {"C2"_us}},
                      {"C2"_us, nullptr, {}, {"R1"_us}, {"Join"_us}, {}},
                  },
                  {"Join"_us},
                  {"R1"_us, "R2"_us, "R3"_us}})
               .ExportCollection (),
           {{{"A", {"B1", "B2"}}, {"B1", {"C1"}}, {"C1", {}}, {"B2", {"C2"}}, {"C2", {}}}});
}

TEST_CASE (ReadWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A"_us, nullptr, {"R1"_us}, {"R2"_us}, {}, {"B1"_us, "B2"_us}},
                      {"B1"_us, nullptr, {"R2"_us}, {"R3"_us}, {"Join"_us}, {"C1"_us}},
                      {"C1"_us, nullptr, {"R3"_us}, {"R2"_us}, {"Join"_us}, {}},
                      {"B2"_us, nullptr, {"R2"_us}, {"R1"_us}, {"Join"_us}, {"C2"_us}},
                      {"C2"_us, nullptr, {}, {"R1"_us}, {"Join"_us}, {}},
                  },
                  {"Join"_us},
                  {"R1"_us, "R2"_us, "R3"_us}})
               .ExportCollection (),
           {});
}

TEST_CASE (WriteWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A"_us, nullptr, {"R1"_us}, {"R2"_us}, {}, {"B1"_us, "B2"_us}},
                      {"B1"_us, nullptr, {}, {"R3"_us}, {"Join"_us}, {"C1"_us}},
                      {"C1"_us, nullptr, {"R3"_us}, {"R2"_us}, {"Join"_us}, {}},
                      {"B2"_us, nullptr, {}, {"R2"_us}, {"Join"_us}, {"C2"_us}},
                      {"C2"_us, nullptr, {}, {"R1"_us}, {"Join"_us}, {}},
                  },
                  {"Join"_us},
                  {"R1"_us, "R2"_us, "R3"_us}})
               .ExportCollection (),
           {});
}

TEST_CASE (ResourceNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A"_us, nullptr, {}, {"R1"_us}, {}, {}}, {"B"_us, nullptr, {}, {"R2"_us}, {}, {}}}, {}, {}})
               .ExportCollection (),
           {});
}

TEST_CASE (ReuseTaskRegister)
{
    using namespace Emergence::Flow::Test;
    Emergence::Flow::TaskRegister taskRegister;
    Grow (TRIVIAL_SAFE_RESOURCE_USAGE_SEED, taskRegister);
    Check (taskRegister.ExportCollection (), GetTrivialSafeResourceUsageExpectation ());

    taskRegister.Clear ();
    Grow (COMPLEX_DEPENDENCIES_SEED, taskRegister);
    Check (taskRegister.ExportCollection (), GetComplexDependenciesExpectation ());
}

END_SUITE

BEGIN_SUITE (ExportUnwrappedDependencyMap)

TEST_CASE (CorrectGraph)
{
    const Emergence::Flow::TaskRegister::UnwrappedDependencyMap expected {
        // There is no records for A* tasks, because they have no dependencies.
        {"B1"_us, {"A1"_us, "A2"_us, "A3"_us, "A4"_us}},
        {"B2"_us, {"A1"_us, "A2"_us, "A3"_us, "A4"_us, "A5"_us, "B1"_us}},
    };

    using namespace Emergence::Flow::Test;
    Emergence::Flow::TaskRegister taskRegister;
    Grow (COMPLEX_DEPENDENCIES_SEED, taskRegister);
    CHECK (taskRegister.ExportUnwrappedDependencyMap () == expected);
}

TEST_CASE (MissingDependencies)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[0u].dependsOn.emplace ("MissingDependency");

    Emergence::Flow::TaskRegister taskRegister;
    Grow (seed, taskRegister);
    CHECK (taskRegister.ExportUnwrappedDependencyMap ().empty ());
}

TEST_CASE (CircularDependency)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[0u].dependsOn.emplace ("B2");

    Emergence::Flow::TaskRegister taskRegister;
    Grow (seed, taskRegister);
    CHECK (taskRegister.ExportUnwrappedDependencyMap ().empty ());
}

END_SUITE

BEGIN_SUITE (ExportVisual)

TEST_CASE (TrivialSafeResourceUsageNoResources)
{
    using namespace Emergence::Flow::Test;
    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();

    // Clear out all information about resources.
    expected.subgraphs.erase (expected.subgraphs.begin ());
    expected.edges.clear ();

    Emergence::VisualGraph::Graph result = Grow (TRIVIAL_SAFE_RESOURCE_USAGE_SEED).ExportVisual (false);
    CHECK (result == expected);
}

TEST_CASE (TrivialSafeResourceUsageWithResources)
{
    using namespace Emergence::Flow::Test;
    Emergence::VisualGraph::Graph result = Grow (TRIVIAL_SAFE_RESOURCE_USAGE_SEED).ExportVisual (true);
    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();
    CHECK (result == expected);
}

TEST_CASE (TrivialSafeResourceUsageWithIntroducedCircularDependency)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[3u].dependencyOf.emplace ("A1");
    Emergence::VisualGraph::Graph result = Grow (seed).ExportVisual (true);

    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();
    expected.subgraphs[1u].edges.emplace_back (Emergence::VisualGraph::Edge {"B2", "A1", {}});
    CHECK (result == expected);
}

TEST_CASE (TrivialSafeResourceUsageWithCollision)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[3u].writeAccess.emplace ("R2");
    Emergence::VisualGraph::Graph result = Grow (seed).ExportVisual (true);

    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();
    expected.edges.emplace_back (Emergence::VisualGraph::Edge {
        TaskNodePath ("B2"), ResourceNodePath ("R2"), Emergence::Flow::TaskRegister::VISUAL_WRITE_ACCESS_COLOR});
    CHECK (result == expected);
}

TEST_CASE (VisualGrouping)
{
    using namespace Emergence::Flow::Test;

    Emergence::Flow::TaskRegister taskRegister;

    {
        auto token = taskRegister.OpenVisualGroup ("A");
        taskRegister.RegisterCheckpoint ("AStart"_us);
        taskRegister.RegisterTask ({"AOne"_us, nullptr, {}, {}, {"AStart"_us}, {"AFinish"_us}});
        taskRegister.RegisterTask ({"ATwo"_us, nullptr, {}, {}, {"AStart"_us}, {"AFinish"_us}});
        taskRegister.RegisterCheckpoint ("AFinish"_us);
    }

    {
        auto token = taskRegister.OpenVisualGroup ("B");
        taskRegister.RegisterCheckpoint ("BStart"_us);
        taskRegister.RegisterTask ({"BOne"_us, nullptr, {}, {}, {"BStart"_us}, {}});
        taskRegister.RegisterTask ({"BTwo"_us, nullptr, {}, {}, {"BOne"_us}, {"BFinish"_us}});
        taskRegister.RegisterCheckpoint ("BFinish"_us);
    }

    taskRegister.RegisterTask ({"Common"_us, nullptr, {}, {}, {"AFinish"_us, "BFinish"_us}, {}});
    Emergence::VisualGraph::Graph result = taskRegister.ExportVisual (false);

    Emergence::VisualGraph::Graph expected {{Emergence::Flow::TaskRegister::VISUAL_ROOT_GRAPH_ID},
                                            {},
                                            {
                                                {{Emergence::Flow::TaskRegister::VISUAL_PIPELINE_GRAPH_ID},
                                                 {},
                                                 {
                                                     {"A",
                                                      "A (Group)",
                                                      {},
                                                      {{
                                                          {"AStart", CheckpointNodeLabel ("AStart")},
                                                          {"AFinish", CheckpointNodeLabel ("AFinish")},
                                                          {"AOne", TaskNodeLabel ("AOne")},
                                                          {"ATwo", TaskNodeLabel ("ATwo")},
                                                      }},
                                                      {}},
                                                     {"B",
                                                      "B (Group)",
                                                      {},
                                                      {{
                                                          {"BStart", CheckpointNodeLabel ("BStart")},
                                                          {"BFinish", CheckpointNodeLabel ("BFinish")},
                                                          {"BOne", TaskNodeLabel ("BOne")},
                                                          {"BTwo", TaskNodeLabel ("BTwo")},
                                                      }},
                                                      {}},
                                                 },
                                                 {{
                                                     {"Common", TaskNodeLabel ("Common")},
                                                 }},
                                                 {
                                                     {"A/AStart", "A/AOne", {}},
                                                     {"A/AOne", "A/AFinish", {}},
                                                     {"A/AStart", "A/ATwo", {}},
                                                     {"A/ATwo", "A/AFinish", {}},
                                                     {"B/BStart", "B/BOne", {}},
                                                     {"B/BOne", "B/BTwo", {}},
                                                     {"B/BTwo", "B/BFinish", {}},
                                                     {"A/AFinish", "Common", {}},
                                                     {"B/BFinish", "Common", {}},
                                                 }},
                                            },
                                            {},
                                            {}};
    CHECK (result == expected);
}

END_SUITE
