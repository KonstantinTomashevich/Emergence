#include <bitset>
#include <limits>

#include <Assert/Assert.hpp>

#include <Container/Algorithm.hpp>
#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>

#include <Flow/Constants.hpp>
#include <Flow/TaskRegister.hpp>

#include <Log/Log.hpp>

namespace Emergence::Flow
{
using namespace Memory::Literals;

Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (), "Flow"_us};
    return group;
}

class TaskGraph final
{
public:
    static Container::Optional<TaskGraph> Build (const TaskRegister &_register) noexcept;

    [[nodiscard]] Emergence::Task::Collection ExportCollection () const noexcept;

    [[nodiscard]] TaskRegister::UnwrappedDependencyMap ExportUnwrappedDependencyMap () const noexcept;

private:
    struct Node final
    {
        Memory::UniqueString name;
        Container::Optional<std::size_t> sourceTaskIndex;
        std::bitset<MAX_RESOURCES> readAccess;
        std::bitset<MAX_RESOURCES> writeAccess;
    };

    struct ReachabilityVisitor final
    {
        enum class VisitationState
        {
            UNVISITED,
            WAITING_FOR_RESULTS,
            READY,
        };

        bool VisitGraph (const TaskGraph &_graph) noexcept;

        Container::Vector<VisitationState> nodeStates {GetDefaultAllocationGroup ()};
        Container::Vector<std::bitset<MAX_GRAPH_NODES>> reachable {GetDefaultAllocationGroup ()};

    private:
        bool VisitNode (const TaskGraph &_graph, std::size_t _index) noexcept;
    };

    TaskGraph () = default;

    [[nodiscard]] bool Verify () const noexcept;

    void AddEdgeToCollection (Emergence::Task::Collection &_collection,
                              const Node &_sourceNode,
                              std::size_t _targetIndex) const noexcept;

    const TaskRegister *source = nullptr;
    Container::Vector<Node> nodes {GetDefaultAllocationGroup ()};
    Container::Vector<Container::Vector<std::size_t>> edges {GetDefaultAllocationGroup ()};
};

Container::Optional<TaskGraph> TaskGraph::Build (const TaskRegister &_register) noexcept
{
    bool noErrors = true;
    TaskGraph graph;
    graph.source = &_register;
    Container::HashMap<Memory::UniqueString, std::size_t> nameToNodeIndex {GetDefaultAllocationGroup ()};

    for (const TaskRegister::Checkpoint &checkpoint : _register.checkpoints)
    {
        Node &node = graph.nodes.emplace_back ();
        node.name = checkpoint.name;

        if (!nameToNodeIndex.emplace (checkpoint.name, graph.nodes.size () - 1u).second)
        {
            EMERGENCE_LOG (ERROR, "TaskGraph: Task|Checkpoint name \"", checkpoint.name, "\" is used more than once!");
            noErrors = false;
        }
    }

    Container::HashMap<Memory::UniqueString, std::size_t> resourceNameToIndex {GetDefaultAllocationGroup ()};
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
                EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find read access resource \"", resource, "\" of task \"",
                               task.name, "\"!");
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
                EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find write access resource \"", resource, "\" of task \"",
                               task.name, "\"!");
                noErrors = false;
            }
            else if (node.readAccess.test (iterator->second))
            {
                EMERGENCE_LOG (ERROR, "TaskGraph: Resource \"", resource, "\" of task \"", task.name,
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
            EMERGENCE_LOG (ERROR, "TaskGraph: Task|Checkpoint name \"", task.name, "\" is used more than once!");
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
                EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find target \"", target, "\" of task \"", task.name, "\"!");
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
                EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find dependency \"", dependency, "\" of task \"", task.name,
                               "\"!");
                noErrors = false;
            }
            else
            {
                graph.edges[iterator->second].emplace_back (taskIndex);
            }
        }
    }

    for (const TaskRegister::CheckpointDependency &checkpointDependency : _register.checkpointDependencies)
    {
        if (auto fromIterator = nameToNodeIndex.find (checkpointDependency.from);
            fromIterator != nameToNodeIndex.end ())
        {
            if (auto toIterator = nameToNodeIndex.find (checkpointDependency.to); toIterator != nameToNodeIndex.end ())
            {
                graph.edges[fromIterator->second].emplace_back (toIterator->second);
            }
            else
            {
                EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find checkpoint \"", checkpointDependency.to,
                               "\" that depends on checkpoint \"", checkpointDependency.from, "\"!");
                noErrors = false;
            }
        }
        else
        {
            EMERGENCE_LOG (ERROR, "TaskGraph: Unable to find checkpoint \"", checkpointDependency.from,
                           "\" that is dependency of checkpoint \"", checkpointDependency.to, "\"!");
            noErrors = false;
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
    EMERGENCE_ASSERT (source);
    if (Verify ())
    {
        Emergence::Task::Collection collection;
        for (const Task &task : source->tasks)
        {
            Emergence::Task::Collection::Item &item = collection.tasks.emplace_back ();
            item.name = task.name;
            item.task = task.executor;
        }

        for (std::size_t sourceIndex = 0u; sourceIndex < edges.size (); ++sourceIndex)
        {
            const Container::Vector<std::size_t> &targetIndices = edges[sourceIndex];
            const Node &sourceNode = nodes[sourceIndex];

            // We process only tasks as sources.
            if (sourceNode.sourceTaskIndex)
            {
                for (std::size_t targetIndex : targetIndices)
                {
                    AddEdgeToCollection (collection, sourceNode, targetIndex);
                }
            }
        }

        return collection;
    }

    return {};
}

TaskRegister::UnwrappedDependencyMap TaskGraph::ExportUnwrappedDependencyMap () const noexcept
{
    TaskRegister::UnwrappedDependencyMap map {GetDefaultAllocationGroup ()};
    EMERGENCE_ASSERT (source);
    ReachabilityVisitor visitor;

    if (visitor.VisitGraph (*this))
    {
        for (std::size_t dependencyIndex = 0u; dependencyIndex < nodes.size (); ++dependencyIndex)
        {
            const Node &dependency = nodes[dependencyIndex];
            if (!dependency.sourceTaskIndex)
            {
                // We do not need checkpoints here.
                continue;
            }

            for (std::size_t dependantIndex = 0u; dependantIndex < nodes.size (); ++dependantIndex)
            {
                if (visitor.reachable[dependencyIndex][dependantIndex])
                {
                    const Node &dependant = nodes[dependantIndex];
                    if (!dependant.sourceTaskIndex)
                    {
                        // We do not need checkpoints here.
                        continue;
                    }

                    auto iterator = map.find (dependant.name);
                    if (iterator == map.end ())
                    {
                        iterator = map.emplace (dependant.name, GetDefaultAllocationGroup ()).first;
                    }

                    iterator->second.emplace (dependency.name);
                }
            }
        }
    }

    return map;
}

bool TaskGraph::ReachabilityVisitor::VisitGraph (const TaskGraph &_graph) noexcept
{
    nodeStates.resize (_graph.nodes.size ());
    reachable.resize (_graph.nodes.size ());

    std::fill (nodeStates.begin (), nodeStates.end (), VisitationState::UNVISITED);
    std::fill (reachable.begin (), reachable.end (), 0u);

    for (std::size_t nodeIndex = 0u; nodeIndex < _graph.nodes.size (); ++nodeIndex)
    {
        if (!VisitNode (_graph, nodeIndex))
        {
            return false;
        }
    }

    return true;
}

bool TaskGraph::ReachabilityVisitor::VisitNode (const TaskGraph &_graph, std::size_t _index) noexcept
{
    switch (nodeStates[_index])
    {
    case VisitationState::WAITING_FOR_RESULTS:
        EMERGENCE_LOG (ERROR,
                       "TaskGraph: Circular dependency found during visitation, printing out all nodes in stack.");

        EMERGENCE_LOG (ERROR, _graph.nodes[_index].name);
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
                EMERGENCE_LOG (ERROR, _graph.nodes[_index].name);
                return false;
            }
        }

        nodeStates[_index] = VisitationState::READY;
        [[fallthrough]];
    }

    case VisitationState::READY:
        return true;
    }

    EMERGENCE_ASSERT (false);
    return true;
}

bool TaskGraph::Verify () const noexcept
{
    EMERGENCE_ASSERT (source);

    // Firstly, we need to traverse graph using DFS to find circular dependencies and collect reachability matrix.
    ReachabilityVisitor visitor;
    if (!visitor.VisitGraph (*this))
    {
        return false;
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
                    Container::StringBuilder builder =
                        EMERGENCE_BEGIN_BUILDING_STRING ("TaskGraph: Race condition is possible between tasks \"",
                                                         firstNode.name, "\" and \"", secondNode.name, "\"! ");

                    auto appendCollision = [&builder, this] (const std::bitset<MAX_RESOURCES> &_collision)
                    {
                        bool firstItem = true;
                        for (std::size_t index = 0u; index < _collision.size (); ++index)
                        {
                            if (_collision.test (index))
                            {
                                if (!firstItem)
                                {
                                    builder.Append (", ");
                                }

                                builder.Append (source->resources[index]);
                                firstItem = false;
                            }
                        }
                    };

                    if (readWriteCollision.any ())
                    {
                        builder.Append ("First task reads and second task writes ");
                        appendCollision (readWriteCollision);
                        builder.Append (".");
                    }

                    if (writeReadCollision.any ())
                    {
                        builder.Append ("First task writes and second task reads ");
                        appendCollision (writeReadCollision);
                        builder.Append (".");
                    }

                    if (writeWriteCollision.any ())
                    {
                        builder.Append ("Both tasks write ");
                        appendCollision (writeWriteCollision);
                        builder.Append (".");
                    }

                    EMERGENCE_LOG (ERROR, builder.Get ());
                    anyDataRaces = true;
                }
            }
        }
    }

    return !anyDataRaces;
}

void TaskGraph::AddEdgeToCollection (Emergence::Task::Collection &_collection,
                                     const Node &_sourceNode,
                                     std::size_t _targetIndex) const noexcept
{
    const Node &targetNode = nodes[_targetIndex];

    // Target is task: just add dependency.
    if (targetNode.sourceTaskIndex)
    {
        Container::AddUnique (_collection.tasks[_sourceNode.sourceTaskIndex.value ()].dependantTasksIndices,
                              targetNode.sourceTaskIndex.value ());
    }
    // Target is checkpoint: source should be dependency of all checkpoint targets.
    else
    {
        const Container::Vector<std::size_t> &checkpointTargets = edges[_targetIndex];
        for (std::size_t checkpointTargetIndex : checkpointTargets)
        {
            AddEdgeToCollection (_collection, _sourceNode, checkpointTargetIndex);
        }
    }
}

TaskRegister::VisualGroupNodePlaced::VisualGroupNodePlaced (TaskRegister::VisualGroupNodePlaced &&_other) noexcept
    : parent (_other.parent)
{
    _other.parent = nullptr;
}

TaskRegister::VisualGroupNodePlaced::~VisualGroupNodePlaced () noexcept
{
    if (parent)
    {
        EMERGENCE_ASSERT (parent->currentVisualGroupNode < parent->visualGroupNodes.size ());
        parent->currentVisualGroupNode = parent->visualGroupNodes[parent->currentVisualGroupNode].parentNode;
    }
}

TaskRegister::VisualGroupNodePlaced::VisualGroupNodePlaced (TaskRegister *_parent,
                                                            Container::String _groupName) noexcept
    : parent (_parent)
{
    parent->visualGroupNodes.emplace_back () = {std::move (_groupName), parent->currentVisualGroupNode};
    parent->currentVisualGroupNode = parent->visualGroupNodes.size () - 1u;
}

void TaskRegister::RegisterTask (Task _task) noexcept
{
    AssertNodeNameUniqueness (_task.name);
    tasks.emplace_back (std::move (_task)).visualGroupIndex = currentVisualGroupNode;
}

void TaskRegister::RegisterCheckpoint (Memory::UniqueString _name) noexcept
{
    AssertNodeNameUniqueness (_name);
    checkpoints.emplace_back () = {_name, currentVisualGroupNode};
}

void TaskRegister::RegisterCheckpointDependency (Memory::UniqueString _from, Memory::UniqueString _to) noexcept
{
    checkpointDependencies.emplace_back () = {_from, _to};
}

void TaskRegister::RegisterResource (Memory::UniqueString _name) noexcept
{
    EMERGENCE_ASSERT (std::find (resources.begin (), resources.end (), _name) == resources.end ());
    resources.emplace_back (_name);
}

TaskRegister::VisualGroupNodePlaced TaskRegister::OpenVisualGroup (Container::String _name) noexcept
{
    return VisualGroupNodePlaced {this, std::move (_name)};
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
    Container::HashMap<Memory::UniqueString, std::size_t> groupIndexByName {GetDefaultAllocationGroup ()};

    for (const Checkpoint &checkpoint : checkpoints)
    {
        VisualGraph::Graph &graph = FindVisualGraphForGroup (pipelineGraph, checkpoint.visualGroupIndex);
        VisualGraph::Node &node = graph.nodes.emplace_back ();
        node.id = *checkpoint.name;
        node.label = *checkpoint.name + VISUAL_CHECKPOINT_LABEL_SUFFIX;
        groupIndexByName.emplace (checkpoint.name, checkpoint.visualGroupIndex);
    }

    for (const Task &task : tasks)
    {
        groupIndexByName.emplace (task.name, task.visualGroupIndex);
    }

    auto getPathByName = [this, &groupIndexByName] (Memory::UniqueString _name)
    {
        const std::size_t groupIndex = groupIndexByName[_name];
        return GetVisualGraphPathForGroup (groupIndex) + *_name;
    };

    auto addDependencyEdge = [&pipelineGraph] (const Container::String &_from, const Container::String &_to)
    {
        VisualGraph::Edge &edge = pipelineGraph.edges.emplace_back ();
        edge.from = _from;
        edge.to = _to;
    };

    for (const Task &task : tasks)
    {
        VisualGraph::Graph &graph = FindVisualGraphForGroup (pipelineGraph, task.visualGroupIndex);
        Container::String path = GetVisualGraphPathForGroup (task.visualGroupIndex) + *task.name;
        VisualGraph::Node &node = graph.nodes.emplace_back ();
        node.id = *task.name;
        node.label = *task.name + VISUAL_TASK_LABEL_SUFFIX;

        if (_exportResources)
        {
            auto addResourceEdge = [&root, &path] (Memory::UniqueString _resource) -> VisualGraph::Edge &
            {
                VisualGraph::Edge &edge = root.edges.emplace_back ();
                edge.from = VISUAL_PIPELINE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR += path;
                edge.to = VISUAL_RESOURCE_GRAPH_ID + VisualGraph::NODE_PATH_SEPARATOR + *_resource;
                return edge;
            };

            for (Memory::UniqueString resource : task.readAccess)
            {
                addResourceEdge (resource).color = VISUAL_READ_ACCESS_COLOR;
            }

            for (Memory::UniqueString resource : task.writeAccess)
            {
                addResourceEdge (resource).color = VISUAL_WRITE_ACCESS_COLOR;
            }
        }

        for (Memory::UniqueString dependency : task.dependsOn)
        {
            addDependencyEdge (getPathByName (dependency), path);
        }

        for (Memory::UniqueString target : task.dependencyOf)
        {
            addDependencyEdge (path, getPathByName (target));
        }
    }

    for (const CheckpointDependency &checkpointDependency : checkpointDependencies)
    {
        addDependencyEdge (getPathByName (checkpointDependency.from), getPathByName (checkpointDependency.to));
    }

    return root;
}

TaskRegister::UnwrappedDependencyMap TaskRegister::ExportUnwrappedDependencyMap () const noexcept
{
    if (Container::Optional<TaskGraph> graph = TaskGraph::Build (*this))
    {
        return graph.value ().ExportUnwrappedDependencyMap ();
    }

    return UnwrappedDependencyMap {GetDefaultAllocationGroup ()};
}

Emergence::Task::Collection TaskRegister::ExportCollection () const noexcept
{
    if (Container::Optional<TaskGraph> graph = TaskGraph::Build (*this))
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
    checkpointDependencies.clear ();
}

// If asserts are not enabled, CLang Tidy advises to convert this function to static, which is not correct.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void TaskRegister::AssertNodeNameUniqueness ([[maybe_unused]] Memory::UniqueString _name) const noexcept
{
#if defined(EMERGENCE_ASSERT_ENABLED)
    auto taskIterator = Container::FindIf (tasks.begin (), tasks.end (),
                                           [_name] (const Task &_task)
                                           {
                                               return _task.name == _name;
                                           });

    if (taskIterator != tasks.end ())
    {
        EMERGENCE_LOG (CRITICAL_ERROR, "TaskGraph: Task name \"", _name, "\" is already occupied by other task!");
        EMERGENCE_ASSERT (false);
    }

    auto checkpointIterator = Container::FindIf (checkpoints.begin (), checkpoints.end (),
                                                 [_name] (const Checkpoint &_checkpoint)
                                                 {
                                                     return _checkpoint.name == _name;
                                                 });

    if (checkpointIterator != checkpoints.end ())
    {
        EMERGENCE_LOG (CRITICAL_ERROR, "TaskGraph: Task name \"", _name, "\" is already occupied by other checkpoint!");
        EMERGENCE_ASSERT (false);
    }
#endif
}

VisualGraph::Graph &TaskRegister::FindVisualGraphForGroup (VisualGraph::Graph &_root,
                                                           std::size_t _groupIndex) const noexcept
{
    VisualGraph::Graph *targetGraph;
    if (_groupIndex == std::numeric_limits<std::size_t>::max ())
    {
        return _root;
    }

    if (visualGroupNodes[_groupIndex].parentNode == std::numeric_limits<std::size_t>::max ())
    {
        targetGraph = &_root;
    }
    else
    {
        targetGraph = &FindVisualGraphForGroup (_root, visualGroupNodes[_groupIndex].parentNode);
    }

    for (VisualGraph::Graph &subgraph : targetGraph->subgraphs)
    {
        if (subgraph.id == visualGroupNodes[_groupIndex].groupName)
        {
            return subgraph;
        }
    }

    VisualGraph::Graph &newGraph = targetGraph->subgraphs.emplace_back ();
    newGraph.id = visualGroupNodes[_groupIndex].groupName;
    newGraph.label = visualGroupNodes[_groupIndex].groupName + VISUAL_GROUP_LABEL_SUFFIX;
    return newGraph;
}

Container::String TaskRegister::GetVisualGraphPathForGroup (std::size_t _groupIndex) const noexcept
{
    if (_groupIndex == std::numeric_limits<std::size_t>::max ())
    {
        return "";
    }

    return GetVisualGraphPathForGroup (visualGroupNodes[_groupIndex].parentNode) +
           visualGroupNodes[_groupIndex].groupName + VisualGraph::NODE_PATH_SEPARATOR;
}
} // namespace Emergence::Flow
