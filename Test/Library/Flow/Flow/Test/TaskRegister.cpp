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

TaskRegister Grow (const Seed &_seed)
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

    return taskRegister;
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

static void Check (const Emergence::Task::Collection &_result, const CollectionExpectation &_expectation)
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
            CHECK_EQUAL (_expectation.tasks[resultIndex].name, expectation.dependencies[dependencyIndex]);
        }
    }
}

static std::string CheckpointNodeLabel (std::string _name)
{
    return _name += TaskRegister::VISUAL_CHECKPOINT_LABEL_SUFFIX;
}

static std::string TaskNodeLabel (std::string _name)
{
    return _name += TaskRegister::VISUAL_TASK_LABEL_SUFFIX;
}

static std::string ResourceNodePath (const char *_name)
{
    return TaskRegister::VISUAL_RESOURCE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _name;
}

static std::string TaskNodePath (const char *_name)
{
    return TaskRegister::VISUAL_PIPELINE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _name;
}

static const Seed TRIVIAL_SAFE_RESOURCE_USAGE_SEED {{
                                                        {"A1", nullptr, {"R1"}, {"R2"}, {}, {"Join"}},
                                                        {"A2", nullptr, {"R1"}, {"R3"}, {}, {"Join"}},
                                                        {"B1", nullptr, {"R2"}, {"R1"}, {"Join"}, {}},
                                                        {"B2", nullptr, {}, {"R3"}, {"Join"}, {}},
                                                    },
                                                    {"Join"},
                                                    {"R1", "R2", "R3"}};

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
} // namespace Emergence::Flow::Test

BEGIN_SUITE (ExportCollection)

TEST_CASE (Sequence)
{
    using namespace Emergence::Flow::Test;
    Check (
        Grow ({{{"A", nullptr, {}, {}, {}, {"B"}}, {"B", nullptr, {}, {}, {}, {}}, {"C", nullptr, {}, {}, {"B"}, {}}},
               {},
               {}})
            .ExportCollection (),
        {{{"A", {}}, {"B", {"A"}}, {"C", {"B"}}}});
}

TEST_CASE (SequenceWithCheckpoint)
{
    using namespace Emergence::Flow::Test;
    Check (
        Grow ({{{"A", nullptr, {}, {}, {}, {"B"}}, {"C", nullptr, {}, {}, {"B"}, {}}}, {"B"}, {}}).ExportCollection (),
        {{{"A", {}}, {"C", {"A"}}}});
}

TEST_CASE (CheckpointAsBarrier)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A1", nullptr, {}, {}, {}, {"Join"}},
                      {"A2", nullptr, {}, {}, {}, {"Join"}},
                      {"B1", nullptr, {}, {}, {"Join"}, {}},
                      {"B2", nullptr, {}, {}, {"Join"}, {}},
                  },
                  {"Join"},
                  {}})
               .ExportCollection (),
           {{{"A1", {}}, {"A2", {}}, {"B1", {"A1", "A2"}}, {"B2", {"A1", "A2"}}}});
}

TEST_CASE (ComplexDependencies)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A1", nullptr, {}, {}, {}, {"Join1"}},
                      {"A2", nullptr, {}, {}, {}, {"Join1"}},
                      {"A3", nullptr, {}, {}, {}, {"Join2"}},
                      {"A4", nullptr, {}, {}, {}, {"Join2"}},
                      {"A5", nullptr, {}, {}, {}, {}},
                      {"B1", nullptr, {}, {}, {"Join1", "Join2"}, {}},
                      {"B2", nullptr, {}, {}, {"Join1", "A5", "B1"}, {}},
                  },
                  {"Join1", "Join2"},
                  {}})
               .ExportCollection (),
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
        Grow (
            {{{"A", nullptr, {}, {}, {}, {"B"}}, {"B", nullptr, {}, {}, {}, {"C"}}, {"C", nullptr, {}, {}, {}, {"A"}}},
             {},
             {}})
            .ExportCollection (),
        {});
}

TEST_CASE (ComplexCycle)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A1", nullptr, {}, {}, {"B2"}, {"Join1"}},
                      {"A2", nullptr, {}, {}, {}, {"Join1"}},
                      {"A3", nullptr, {}, {}, {}, {"Join2"}},
                      {"A4", nullptr, {}, {}, {}, {"Join2"}},
                      {"A5", nullptr, {}, {}, {}, {}},
                      {"B1", nullptr, {}, {}, {"Join1", "Join2"}, {}},
                      {"B2", nullptr, {}, {}, {"Join1", "A5", "B1"}, {}},
                  },
                  {"Join1", "Join2"},
                  {}})
               .ExportCollection (),
           {});
}

TEST_CASE (DependencyNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A", nullptr, {}, {}, {}, {"D"}}, {"B", nullptr, {}, {}, {"Join2"}, {}}}, {"Join1"}, {}})
               .ExportCollection (),
           {});
}

TEST_CASE (DuplicateResourceUsageInAccessMask)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{{"A", nullptr, {"R1"}, {"R1"}, {}, {}}}, {}, {"R1"}}).ExportCollection (), {});
}

TEST_CASE (TrivialSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (Grow (TRIVIAL_SAFE_RESOURCE_USAGE_SEED).ExportCollection (),
           {{{"A1", {}}, {"A2", {}}, {"B1", {"A1", "A2"}}, {"B2", {"A1", "A2"}}}});
}

TEST_CASE (ChainedSafeResourceUsage)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                      {"B1", nullptr, {"R2"}, {"R3"}, {"Join"}, {"C1"}},
                      {"C1", nullptr, {"R3"}, {}, {"Join"}, {}},
                      {"B2", nullptr, {"R2"}, {"R1"}, {"Join"}, {"C2"}},
                      {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                  },
                  {"Join"},
                  {"R1", "R2", "R3"}})
               .ExportCollection (),
           {{{"A", {}}, {"B1", {"A"}}, {"C1", {"B1"}}, {"B2", {"A"}}, {"C2", {"B2"}}}});
}

TEST_CASE (ReadWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                      {"B1", nullptr, {"R2"}, {"R3"}, {"Join"}, {"C1"}},
                      {"C1", nullptr, {"R3"}, {"R2"}, {"Join"}, {}},
                      {"B2", nullptr, {"R2"}, {"R1"}, {"Join"}, {"C2"}},
                      {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                  },
                  {"Join"},
                  {"R1", "R2", "R3"}})
               .ExportCollection (),
           {});
}

TEST_CASE (WriteWriteCollision)
{
    using namespace Emergence::Flow::Test;
    Check (Grow ({{
                      {"A", nullptr, {"R1"}, {"R2"}, {}, {"B1", "B2"}},
                      {"B1", nullptr, {}, {"R3"}, {"Join"}, {"C1"}},
                      {"C1", nullptr, {"R3"}, {"R2"}, {"Join"}, {}},
                      {"B2", nullptr, {}, {"R2"}, {"Join"}, {"C2"}},
                      {"C2", nullptr, {}, {"R1"}, {"Join"}, {}},
                  },
                  {"Join"},
                  {"R1", "R2", "R3"}})
               .ExportCollection (),
           {});
}

TEST_CASE (ResourceNotExists)
{
    using namespace Emergence::Flow::Test;
    Check (
        Grow ({{{"A", nullptr, {}, {"R1"}, {}, {}}, {"B", nullptr, {}, {"R2"}, {}, {}}}, {}, {}}).ExportCollection (),
        {});
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

TEST_CASE (TrivialSafeResourceUsageWithIntroducedCycle)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[3u].dependencyOf.emplace_back ("A1");
    Emergence::VisualGraph::Graph result = Grow (seed).ExportVisual (true);

    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();
    expected.subgraphs[1u].edges.emplace_back (Emergence::VisualGraph::Edge {"B2", "A1", {}});
    CHECK (result == expected);
}

TEST_CASE (TrivialSafeResourceUsageWithCollision)
{
    using namespace Emergence::Flow::Test;

    Seed seed = TRIVIAL_SAFE_RESOURCE_USAGE_SEED;
    seed.tasks[3u].writeAccess.emplace_back ("R2");
    Emergence::VisualGraph::Graph result = Grow (seed).ExportVisual (true);

    Emergence::VisualGraph::Graph expected = GetTrivialSafeResourceUsageGraph ();
    expected.edges.emplace_back (Emergence::VisualGraph::Edge {
        TaskNodePath ("B2"), ResourceNodePath ("R2"), Emergence::Flow::TaskRegister::VISUAL_WRITE_ACCESS_COLOR});
    CHECK (result == expected);
}

END_SUITE
