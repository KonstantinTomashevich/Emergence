#include <bitset>
#include <cassert>
#include <optional>
#include <unordered_map>

#include <Flow/Constants.hpp>
#include <Flow/TaskRegister.hpp>

#include <Log/Log.hpp>

namespace Emergence::Flow
{
using namespace Memory::Literals;
using namespace std::string_literals;

Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (), "Flow"_us};
    return group;
}

class TaskGraph final
{
public:
    static std::optional<TaskGraph> Build (const TaskRegister &_register) noexcept;

    [[nodiscard]] Emergence::Task::Collection ExportCollection () const noexcept;

private:
    struct Node final
    {
        Memory::UniqueString name;
        std::optional<std::size_t> sourceTaskIndex;
        std::bitset<MAX_RESOURCES> readAccess;
        std::bitset<MAX_RESOURCES> writeAccess;
    };

    TaskGraph () = default;

    [[nodiscard]] bool Verify () const noexcept;

    const TaskRegister *source = nullptr;
    Container::Vector<Node> nodes {GetDefaultAllocationGroup ()};
    Container::Vector<Container::Vector<std::size_t>> edges {GetDefaultAllocationGroup ()};
};

std::optional<TaskGraph> TaskGraph::Build (const TaskRegister &_register) noexcept
{
    bool noErrors = true;
    TaskGraph graph;
    graph.source = &_register;
    std::unordered_map<Memory::UniqueString, std::size_t> nameToNodeIndex;

    for (Memory::UniqueString checkpoint : _register.checkpoints)
    {
        Node &node = graph.nodes.emplace_back ();
        node.name = checkpoint;

        if (!nameToNodeIndex.emplace (checkpoint, graph.nodes.size () - 1u).second)
        {
            Log::GlobalLogger::Log (Log::Level::ERROR,
                                    "TaskGraph: Task|Checkpoint name \""s + *checkpoint + "\" is used more than once!");
            noErrors = false;
        }
    }

    std::unordered_map<Memory::UniqueString, std::size_t> resourceNameToIndex;
    for (std::size_t index = 0u; index < _register.resources.size (); ++index)
    {
        resourceNameToIndex.emplace (_register.resources[index], index);
    }

    for (std::size_t index = 0u; index < _register.tasks.size (); ++index)
    {
        const Task &task = _register.tasks[index];
        Node &node = graph.nodes.emplace_back ();
        node.name = task.name;
        node.sourceTaskIndex = index;

        for (Memory::UniqueString resource : task.readAccess)
        {
            auto iterator = resourceNameToIndex.find (resource);
            if (iterator == resourceNameToIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find read access resource \""s +
                                                               *resource + "\" of task \"" + *task.name + "\"!");
                noErrors = false;
            }
            else
            {
                node.readAccess.set (iterator->second);
            }
        }

        for (Memory::UniqueString resource : task.writeAccess)
        {
            auto iterator = resourceNameToIndex.find (resource);
            if (iterator == resourceNameToIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find write access resource \""s +
                                                               *resource + "\" of task \"" + *task.name + "\"!");
                noErrors = false;
            }
            else if (node.readAccess.test (iterator->second))
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Resource \""s + *resource + "\" of task \"" +
                                                               *task.name +
                                                               "\" used both in read access in write access lists!");
                noErrors = false;
            }
            else
            {
                node.writeAccess.set (iterator->second);
            }
        }

        if (!nameToNodeIndex.emplace (task.name, graph.nodes.size () - 1u).second)
        {
            Log::GlobalLogger::Log (Log::Level::ERROR,
                                    "TaskGraph: Task|Checkpoint name \""s + *task.name + "\" is used more than once!");
            noErrors = false;
        }
    }

    graph.edges.resize (graph.nodes.size (), Container::Vector<std::size_t> {GetDefaultAllocationGroup ()});
    for (const Task &task : _register.tasks)
    {
        std::size_t taskIndex = nameToNodeIndex.at (task.name);
        for (Memory::UniqueString target : task.dependencyOf)
        {
            auto iterator = nameToNodeIndex.find (target);
            if (iterator == nameToNodeIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find target \""s + *target +
                                                               "\" of task \"" + *task.name + "\"!");
                noErrors = false;
            }
            else
            {
                graph.edges[taskIndex].emplace_back (iterator->second);
            }
        }

        for (Memory::UniqueString dependency : task.dependsOn)
        {
            auto iterator = nameToNodeIndex.find (dependency);
            if (iterator == nameToNodeIndex.end ())
            {
                Log::GlobalLogger::Log (Log::Level::ERROR, "TaskGraph: Unable to find dependency \""s + *dependency +
                                                               "\" of task \"" + *task.name + "\"!");
                noErrors = false;
            }
            else
            {
                graph.edges[iterator->second].emplace_back (taskIndex);
            }
        }
    }

    if (noErrors)
    {
        return graph;
    }

    return std::nullopt;
}

Emergence::Task::Collection TaskGraph::ExportCollection () const noexcept
{
    assert (source);
    if (Verify ())
    {
        Emergence::Task::Collection collection;
        for (const Task &task : source->tasks)
        {
            Emergence::Task::Collection::Item &item = collection.tasks.emplace_back ();
            item.name = task.name;
            item.task = task.executor;
        }

        auto emplaceWithoutDuplication = [] (Container::Vector<std::size_t> &_indices, std::size_t _index)
        {
            if (std::find (_indices.begin (), _indices.end (), _index) == _indices.end ())
            {
                _indices.emplace_back (_index);
            }
        };

        for (std::size_t sourceIndex = 0u; sourceIndex < edges.size (); ++sourceIndex)
        {
            const Container::Vector<std::size_t> &targetIndices = edges[sourceIndex];
            const Node &sourceNode = nodes[sourceIndex];

            // We process only tasks as sources.
            if (sourceNode.sourceTaskIndex)
            {
                for (std::size_t targetIndex : targetIndices)
                {
                    const Node &targetNode = nodes[targetIndex];

                    // Target is task: just add dependency.
                    if (targetNode.sourceTaskIndex)
                    {
                        emplaceWithoutDuplication (
                            collection.tasks[sourceNode.sourceTaskIndex.value ()].dependantTasksIndices,
                            targetNode.sourceTaskIndex.value ());
                    }
                    // Target is checkpoint: source should be dependency of all checkpoint targets.
                    else
                    {
                        const Container::Vector<std::size_t> &checkpointTargets = edges[targetIndex];
                        for (std::size_t checkpointTargetIndex : checkpointTargets)
                        {
                            const Node &checkpointTargetNode = nodes[checkpointTargetIndex];
                            // Checkpoints can not depend on other checkpoints.
                            assert (checkpointTargetNode.sourceTaskIndex);

                            emplaceWithoutDuplication (
                                collection.tasks[sourceNode.sourceTaskIndex.value ()].dependantTasksIndices,
                                checkpointTargetNode.sourceTaskIndex.value ());
                        }
                    }
                }
            }
        }

        return collection;
    }

    return {};
}

bool TaskGraph::Verify () const noexcept
{
    assert (source);

    // Firstly, we need to traverse graph using DFS to find circular dependencies and collect reachability matrix.

    enum class VisitationState
    {
        UNVISITED,
        WAITING_FOR_RESULTS,
        READY,
    };

    struct
    {
        Container::Vector<VisitationState> nodeStates {GetDefaultAllocationGroup ()};
        Container::Vector<std::bitset<MAX_GRAPH_NODES>> reachable {GetDefaultAllocationGroup ()};

        bool VisitNode (const TaskGraph &_graph, std::size_t _index) noexcept
        {
            switch (nodeStates[_index])
            {
            case VisitationState::WAITING_FOR_RESULTS:
                Log::GlobalLogger::Log (
                    Log::Level::ERROR,
                    "TaskGraph: Circular dependency found during visitation, printing out all nodes in stack.");

                Log::GlobalLogger::Log (Log::Level::ERROR, *_graph.nodes[_index].name);
                return false;

            case VisitationState::UNVISITED:
            {
                nodeStates[_index] = VisitationState::WAITING_FOR_RESULTS;
                const Container::Vector<std::size_t> &nodeEdges = _graph.edges[_index];

                for (std::size_t child : nodeEdges)
                {
                    reachable[_index].set (child);
                    if (VisitNode (_graph, child))
                    {
                        reachable[_index] |= reachable[child];
                    }
                    else
                    {
                        Log::GlobalLogger::Log (Log::Level::ERROR, *_graph.nodes[_index].name);
                        return false;
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
    } visitor;

    visitor.nodeStates.resize (nodes.size (), VisitationState::UNVISITED);
    visitor.reachable.resize (nodes.size ());

    for (std::size_t nodeIndex = 0u; nodeIndex < nodes.size (); ++nodeIndex)
    {
        if (!visitor.VisitNode (*this, nodeIndex))
        {
            return false;
        }
    }

    // Graph contains no circular dependencies, therefore we can search
    // for possible data races using access masks and reachability matrix.
    bool anyDataRaces = false;

    for (std::size_t firstNodeIndex = 0u; firstNodeIndex < nodes.size (); ++firstNodeIndex)
    {
        const Node &firstNode = nodes[firstNodeIndex];
        for (std::size_t secondNodeIndex = firstNodeIndex + 1u; secondNodeIndex < nodes.size (); ++secondNodeIndex)
        {
            const Node &secondNode = nodes[secondNodeIndex];
            const bool canBeExecutedSimultaneously = !visitor.reachable[firstNodeIndex].test (secondNodeIndex) &&
                                                     !visitor.reachable[secondNodeIndex].test (firstNodeIndex);

            if (canBeExecutedSimultaneously)
            {
                const std::bitset<MAX_RESOURCES> readWriteCollision = firstNode.readAccess & secondNode.writeAccess;
                const std::bitset<MAX_RESOURCES> writeReadCollision = firstNode.writeAccess & secondNode.readAccess;
                const std::bitset<MAX_RESOURCES> writeWriteCollision = firstNode.writeAccess & secondNode.writeAccess;

                if (readWriteCollision.any () || writeReadCollision.any () || writeWriteCollision.any ())
                {
                    std::string error = "TaskGraph: Race condition is possible between tasks \""s + *firstNode.name +
                                        "\" and \"" + *secondNode.name + "\"! ";

                    auto appendCollision = [&error, this] (const std::bitset<MAX_RESOURCES> &_collision)
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

                                error.append (*source->resources[index]);
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
                        error.append ("Both tasks write ");
                        appendCollision (writeWriteCollision);
                        error.append (".");
                    }

                    Log::GlobalLogger::Log (Log::Level::ERROR, error);
                    anyDataRaces = true;
                }
            }
        }
    }

    return !anyDataRaces;
}

void TaskRegister::RegisterTask (Task _task) noexcept
{
    AssertNodeNameUniqueness (_task.name);
    tasks.emplace_back (std::move (_task));
}

void TaskRegister::RegisterCheckpoint (Memory::UniqueString _name) noexcept
{
    AssertNodeNameUniqueness (_name);
    checkpoints.emplace_back (_name);
}

void TaskRegister::RegisterResource (Memory::UniqueString _name) noexcept
{
    assert (std::find (resources.begin (), resources.end (), _name) == resources.end ());
    resources.emplace_back (_name);
}

VisualGraph::Graph TaskRegister::ExportVisual (bool _exportResources) const noexcept
{
    VisualGraph::Graph root;
    root.id = VISUAL_ROOT_GRAPH_ID;

    if (_exportResources)
    {
        VisualGraph::Graph &resourceGraph = root.subgraphs.emplace_back ();
        resourceGraph.id = VISUAL_RESOURCE_GRAPH_ID;

        for (Memory::UniqueString resource : resources)
        {
            VisualGraph::Node &node = resourceGraph.nodes.emplace_back ();
            node.id = *resource;
        }
    }

    VisualGraph::Graph &pipelineGraph = root.subgraphs.emplace_back ();
    pipelineGraph.id = VISUAL_PIPELINE_GRAPH_ID;

    for (Memory::UniqueString checkpoint : checkpoints)
    {
        VisualGraph::Node &node = pipelineGraph.nodes.emplace_back ();
        node.id = *checkpoint;
        node.label = *checkpoint + VISUAL_CHECKPOINT_LABEL_SUFFIX;
    }

    for (const Task &task : tasks)
    {
        VisualGraph::Node &node = pipelineGraph.nodes.emplace_back ();
        node.id = *task.name;
        node.label = *task.name + VISUAL_TASK_LABEL_SUFFIX;

        if (_exportResources)
        {
            auto addResourceEdge = [&root] (const std::string &_task,
                                            Memory::UniqueString _resource) -> VisualGraph::Edge &
            {
                VisualGraph::Edge &edge = root.edges.emplace_back ();
                edge.from = VISUAL_PIPELINE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR += _task;
                edge.to = VISUAL_RESOURCE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + *_resource;
                return edge;
            };

            for (Memory::UniqueString resource : task.readAccess)
            {
                addResourceEdge (node.id, resource).color = VISUAL_READ_ACCESS_COLOR;
            }

            for (Memory::UniqueString resource : task.writeAccess)
            {
                addResourceEdge (node.id, resource).color = VISUAL_WRITE_ACCESS_COLOR;
            }
        }

        auto addDependencyEdge = [&pipelineGraph] (const std::string &_from, const std::string &_to)
        {
            VisualGraph::Edge &edge = pipelineGraph.edges.emplace_back ();
            edge.from = _from;
            edge.to = _to;
        };

        for (Memory::UniqueString dependency : task.dependsOn)
        {
            addDependencyEdge (std::string {*dependency}, node.id);
        }

        for (Memory::UniqueString target : task.dependencyOf)
        {
            addDependencyEdge (node.id, std::string {*target});
        }
    }

    return root;
}

Emergence::Task::Collection TaskRegister::ExportCollection () const noexcept
{
    if (std::optional<TaskGraph> graph = TaskGraph::Build (*this))
    {
        return graph.value ().ExportCollection ();
    }

    return {};
}

void TaskRegister::Clear () noexcept
{
    tasks.clear ();
    checkpoints.clear ();
    resources.clear ();
}

// If asserts are not enabled, CLang Tidy advises to convert this function to static, which is not correct.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void TaskRegister::AssertNodeNameUniqueness ([[maybe_unused]] Memory::UniqueString _name) const noexcept
{
    assert (std::find_if (tasks.begin (), tasks.end (),
                          [_name] (const Task &_task)
                          {
                              return _task.name == _name;
                          }) == tasks.end ());

    assert (std::find (checkpoints.begin (), checkpoints.end (), _name) == checkpoints.end ());
}
} // namespace Emergence::Flow
