#include <bitset>
#include <cassert>
#include <optional>
#include <unordered_map>

#include <Flow/Constants.hpp>
#include <Flow/TaskRegister.hpp>

#include <Log/Log.hpp>

namespace Emergence::Flow
{
struct GraphNode final
{
    std::string name;
    std::optional<std::size_t> sourceTaskIndex;
    std::bitset<MAX_RESOURCES> readAccess;
    std::bitset<MAX_RESOURCES> writeAccess;
};

struct Graph final
{
    std::vector<GraphNode> nodes;
    std::unordered_map<std::size_t, std::vector<std::size_t>> edges;
};

enum class VisitationState
{
    UNVISITED,
    WAITING_FOR_RESULTS,
    READY,
};

struct GraphVisitor
{
    std::vector<VisitationState> nodeStates;
    std::vector<std::bitset<MAX_GRAPH_NODES>> reachable;

    void Prepare (const Graph &_graph) noexcept;

    bool VisitNode (const Graph &_graph, std::size_t _index) noexcept;
};

void GraphVisitor::Prepare (const Graph &_graph) noexcept
{
    nodeStates.clear ();
    nodeStates.resize (_graph.nodes.size (), VisitationState::UNVISITED);

    reachable.clear ();
    reachable.resize (_graph.nodes.size ());
}

bool GraphVisitor::VisitNode (const Graph &_graph, std::size_t _index) noexcept
{
    switch (nodeStates[_index])
    {
    case VisitationState::WAITING_FOR_RESULTS:
        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "TaskGraph: Cycle found during visitation, printing out all nodes in stack.");
        Log::GlobalLogger::Log (Log::Level::ERROR, _graph.nodes[_index].name);
        return false;

    case VisitationState::UNVISITED:
    {
        nodeStates[_index] = VisitationState::WAITING_FOR_RESULTS;
        auto edges = _graph.edges.find (_index);

        if (edges != _graph.edges.end ())
        {
            for (std::size_t child : edges->second)
            {
                reachable[_index].set (child);
                if (VisitNode (_graph, child))
                {
                    reachable[_index] |= reachable[child];
                }
                else
                {
                    Log::GlobalLogger::Log (Log::Level::ERROR, _graph.nodes[_index].name);
                    return false;
                }
            }
        }

        nodeStates[_index] = VisitationState::READY;
        [[fallthrough]];
    }

    case VisitationState::READY:
        return true;
    }

    assert (false);
    return true;
}

bool EnsureConcurrencySafety (const Graph &_graph,
                              const std::vector<std::bitset<MAX_GRAPH_NODES>> &_reachable,
                              const std::vector<std::string> &_resourceNames) noexcept
{
    bool safe = true;
    for (std::size_t primaryNodeIndex = 0u; primaryNodeIndex < _graph.nodes.size (); ++primaryNodeIndex)
    {
        const GraphNode &primaryNode = _graph.nodes[primaryNodeIndex];
        for (std::size_t secondaryNodeIndex = primaryNodeIndex + 1u; secondaryNodeIndex < _graph.nodes.size ();
             ++secondaryNodeIndex)
        {
            const GraphNode &secondaryNode = _graph.nodes[secondaryNodeIndex];
            const std::bitset<MAX_RESOURCES> readWriteCollision = primaryNode.readAccess & secondaryNode.writeAccess;
            const std::bitset<MAX_RESOURCES> writeReadCollision = primaryNode.writeAccess & secondaryNode.readAccess;
            const std::bitset<MAX_RESOURCES> writeWriteCollision = primaryNode.writeAccess & secondaryNode.writeAccess;

            if (readWriteCollision.any () || writeReadCollision.any () || writeWriteCollision.any ())
            {
                const bool preventedThroughDependencies = _reachable[primaryNodeIndex].test (secondaryNodeIndex) ||
                                                          _reachable[secondaryNodeIndex].test (primaryNodeIndex);

                if (!preventedThroughDependencies)
                {
                    std::string error = "TaskGraph: Race condition is possible between tasks \"" + primaryNode.name +
                                        "\" and \"" + secondaryNode.name + "\"! ";

                    auto appendCollision = [&error, &_resourceNames] (const std::bitset<MAX_RESOURCES> &_collision)
                    {
                        bool firstItem = true;
                        for (std::size_t index = 0u; index < _collision.size (); ++index)
                        {
                            if (_collision.test (index))
                            {
                                if (!firstItem)
                                {
                                    error.append (", ");
                                }

                                error.append (_resourceNames[index]);
                                firstItem = false;
                            }
                        }
                    };

                    if (readWriteCollision.any ())
                    {
                        error.append ("First task reads and second task writes ");
                        appendCollision (readWriteCollision);
                        error.append (".");
                    }

                    if (writeReadCollision.any ())
                    {
                        error.append ("First task writes and second task reads ");
                        appendCollision (writeReadCollision);
                        error.append (".");
                    }

                    if (writeWriteCollision.any ())
                    {
                        error.append ("First task writes and second task writes ");
                        appendCollision (writeWriteCollision);
                        error.append (".");
                    }

                    Log::GlobalLogger::Log (Log::Level::ERROR, error);
                    safe = false;
                }
            }
        }
    }

    return safe;
}

void TaskRegister::AddTask (Task _task) noexcept
{
    AssertNodeNameUniqueness (_task.name.c_str ());
    tasks.emplace_back (std::move (_task));
}

void TaskRegister::RegisterCheckpoint (const char *_name) noexcept
{
    AssertNodeNameUniqueness (_name);
    checkpoints.emplace_back (_name);
}

void TaskRegister::RegisterResource (const char *_name) noexcept
{
    assert (std::find (resources.begin (), resources.end (), _name) == resources.end ());
    resources.emplace_back (_name);
}

VisualGraph::Graph TaskRegister::ExportVisual (bool _exportResources) const noexcept
{
    constexpr const char *ROOT_ID = "TaskGraph";
    constexpr const char *RESOURCE_GRAPH_ID = "Resources";
    constexpr const char *PIPELINE_GRAPH_ID = "Pipeline";

    constexpr const char *READ_ACCESS_COLOR = "#0000FFFF";
    constexpr const char *WRITE_ACCESS_COLOR = "#FF0000FF";

    VisualGraph::Graph root;
    root.id = ROOT_ID;

    if (_exportResources)
    {
        VisualGraph::Graph &resourceGraph = root.subgraphs.emplace_back ();
        resourceGraph.id = RESOURCE_GRAPH_ID;

        for (const std::string &resource : resources)
        {
            VisualGraph::Node &node = resourceGraph.nodes.emplace_back ();
            node.id = resource;
        }
    }

    VisualGraph::Graph &pipelineGraph = root.subgraphs.emplace_back ();
    pipelineGraph.id = PIPELINE_GRAPH_ID;

    for (const std::string &checkpoint : checkpoints)
    {
        VisualGraph::Node &node = pipelineGraph.nodes.emplace_back ();
        node.id = checkpoint;
        node.label = checkpoint + " (Checkpoint)";
    }

    for (const Task &task : tasks)
    {
        VisualGraph::Node &node = pipelineGraph.nodes.emplace_back ();
        node.id = task.name;
        node.label = task.name + " (Task)";

        if (_exportResources)
        {
            auto addResourceEdge = [&root] (const std::string &_task, const std::string &_resource)
            {
                VisualGraph::Edge &edge = root.edges.emplace_back ();
                edge.from = PIPELINE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _task;
                edge.to = RESOURCE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + _resource;
                return edge;
            };

            for (const std::string &resource : task.readAccess)
            {
                addResourceEdge (node.id, resource).color = READ_ACCESS_COLOR;
            }

            for (const std::string &resource : task.writeAccess)
            {
                addResourceEdge (node.id, resource).color = WRITE_ACCESS_COLOR;
            }
        }

        auto addDependencyEdge = [&pipelineGraph] (const std::string &_from, const std::string &_to)
        {
            VisualGraph::Edge &edge = pipelineGraph.edges.emplace_back ();
            edge.from = _from;
            edge.to = _to;
        };

        for (const std::string &dependency : task.dependsOn)
        {
            addDependencyEdge (dependency, node.id);
        }

        for (const std::string &target : task.dependencyOf)
        {
            addDependencyEdge (node.id, target);
        }
    }

    return root;
}

TaskCollection TaskRegister::ExportCollection () const noexcept
{
    Graph graph;
    if (!BuildGraph (&graph))
    {
        return {};
    }

    GraphVisitor visitor;
    for (std::size_t index = 0u; index < graph.nodes.size (); ++index)
    {
        if (!visitor.VisitNode (graph, index))
        {
            return {};
        }
    }

    if (!EnsureConcurrencySafety (graph, visitor.reachable, resources))
    {
        return {};
    }

    TaskCollection collection;
    for (const Task &task : tasks)
    {
        TaskCollection::Item &item = collection.tasks.emplace_back ();
        item.name = task.name;
        item.task = task.executor;
    }

    auto emplaceWithoutDuplication = [] (std::vector<std::size_t> &_indices, std::size_t _index)
    {
        if (std::find (_indices.begin (), _indices.end (), _index) == _indices.end ())
        {
            _indices.emplace_back (_index);
        }
    };

    for (const auto &[sourceIndex, targetIndices] : graph.edges)
    {
        const GraphNode &source = graph.nodes[sourceIndex];

        // We process only tasks as sources.
        if (source.sourceTaskIndex)
        {
            for (std::size_t targetIndex : targetIndices)
            {
                const GraphNode &target = graph.nodes[targetIndex];

                // Target is task: just add dependency.
                if (target.sourceTaskIndex)
                {
                    emplaceWithoutDuplication (collection.tasks[target.sourceTaskIndex.value ()].dependencyIndices,
                                               source.sourceTaskIndex.value ());
                }
                // Target is checkpoint: source should be dependency of all checkpoint targets.
                else
                {
                    std::vector<std::size_t> &checkpointTargets = graph.edges[targetIndex];
                    for (std::size_t checkpointTargetIndex : checkpointTargets)
                    {
                        const GraphNode &checkpointTarget = graph.nodes[checkpointTargetIndex];
                        // Checkpoints can not depend on other checkpoints.
                        assert (checkpointTarget.sourceTaskIndex);

                        emplaceWithoutDuplication (
                            collection.tasks[checkpointTarget.sourceTaskIndex.value ()].dependencyIndices,
                            source.sourceTaskIndex.value ());
                    }
                }
            }
        }
    }

    return collection;
}

// If asserts are not enabled, CLang Tidy advises to convert this function to static, which is not correct.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void TaskRegister::AssertNodeNameUniqueness ([[maybe_unused]] const char *_name) const noexcept
{
    assert (std::find_if (tasks.begin (), tasks.end (),
                          [_name] (const Task &_task)
                          {
                              return _task.name == _name;
                          }) == tasks.end ());

    assert (std::find (checkpoints.begin (), checkpoints.end (), _name) == checkpoints.end ());
}

bool TaskRegister::BuildGraph (struct Graph *_graph) const noexcept
{
    bool noErrors = true;
    std::unordered_map<std::string, std::size_t> nameToNodeIndex;

    for (const std::string &checkpoint : checkpoints)
    {
        GraphNode &node = _graph->nodes.emplace_back ();
        node.name = checkpoint;

        if (!nameToNodeIndex.emplace (checkpoint, _graph->nodes.size () - 1u).second)
        {
            Log::GlobalLogger::Log (Log::Level::ERROR,
                                    "TaskGraph: Task|Checkpoint name \"" + checkpoint + "\" is used more than once!");
            noErrors = false;
        }
    }

    std::unordered_map<std::string, std::size_t> resourceNameToIndex;
    for (std::size_t index = 0u; index < resources.size (); ++index)
    {
        resourceNameToIndex.emplace (resources[index], index);
    }

    for (std::size_t index = 0u; index < tasks.size (); ++index)
    {
        const Task &task = tasks[index];
        GraphNode &node = _graph->nodes.emplace_back ();
        node.name = task.name;
        node.sourceTaskIndex = index;

        for (const std::string &resource : task.readAccess)
        {
            auto iterator = resourceNameToIndex.find (resource);
            if (iterator == resourceNameToIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find read access resource \"" +
                                                               resource + "\" of task \"" + task.name + "\"!");
                noErrors = false;
            }

            node.readAccess.set (iterator->second);
        }

        for (const std::string &resource : task.writeAccess)
        {
            auto iterator = resourceNameToIndex.find (resource);
            if (iterator == resourceNameToIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find write access resource \"" +
                                                               resource + "\" of task \"" + task.name + "\"!");
                noErrors = false;
            }

            if (node.readAccess.test (iterator->second))
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Resource \"" + resource + "\" of task \"" +
                                                               task.name +
                                                               "\" used both in read access in write access lists!");
                noErrors = false;
            }

            node.writeAccess.set (iterator->second);
        }

        if (!nameToNodeIndex.emplace (task.name, _graph->nodes.size () - 1u).second)
        {
            Log::GlobalLogger::Log (Log::Level::ERROR,
                                    "TaskGraph: Task|Checkpoint name \"" + task.name + "\" is used more than once!");
            noErrors = false;
        }
    }

    for (const Task &task : tasks)
    {
        std::size_t taskIndex = nameToNodeIndex.at (task.name);
        for (const std::string &target : task.dependencyOf)
        {
            auto iterator = nameToNodeIndex.find (target);
            if (iterator == nameToNodeIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find target \"" + target +
                                                               "\" of task \"" + task.name + "\"!");
                noErrors = false;
            }
            else
            {
                _graph->edges[taskIndex].emplace_back (iterator->second);
            }
        }

        for (const std::string &dependency : task.dependsOn)
        {
            auto iterator = nameToNodeIndex.find (dependency);
            if (iterator == nameToNodeIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find dependency \"" + dependency +
                                                               "\" of task \"" + task.name + "\"!");
                noErrors = false;
            }
            else
            {
                _graph->edges[iterator->second].emplace_back (taskIndex);
            }
        }
    }

    return noErrors;
}
} // namespace Emergence::Flow
