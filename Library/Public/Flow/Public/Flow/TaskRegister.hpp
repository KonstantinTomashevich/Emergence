#pragma once

#include <functional>

#include <API/Common/Shortcuts.hpp>

#include <Container/HashSet.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/UniqueString.hpp>

#include <Task/Collection.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Flow
{
Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept;

/// \brief Contains all useful information about task.
struct Task
{
    /// \brief Task name is used for dependency connection, logging and debugging.
    /// \invariant Must be unique among tasks and checkpoints.
    Memory::UniqueString name;

    /// \see TaskCollection::Item::task
    std::function<void ()> executor;

    // TODO: Ideally we should use flat sets here, because item count is rather small.

    /// \brief Names of resources, that are read by this task.
    Container::HashSet<Memory::UniqueString> readAccess {GetDefaultAllocationGroup ()};

    /// \brief Names of resources, that are modified by this task.
    /// \invariant If task both reads and modifies one resources, this resource should only be added to ::writeAccess.
    Container::HashSet<Memory::UniqueString> writeAccess {GetDefaultAllocationGroup ()};

    /// \brief Names of tasks, on which this task depends.
    Container::HashSet<Memory::UniqueString> dependsOn {GetDefaultAllocationGroup ()};

    /// \brief Names of tasks, to which this task injects itself as dependency.
    Container::HashSet<Memory::UniqueString> dependencyOf {GetDefaultAllocationGroup ()};
};

/// \brief Allows user to register tasks and export result as task collection or visual graph.
/// \details Registration order doesn't matter: all verification will be done during ::ExportCollection.
class TaskRegister final
{
public:
    // We use classic strings here, because there is no need to waste unique string stack for visualization-only data.
    inline static const Container::String VISUAL_ROOT_GRAPH_ID = "TaskGraph";
    inline static const Container::String VISUAL_RESOURCE_GRAPH_ID = "Resources";
    inline static const Container::String VISUAL_PIPELINE_GRAPH_ID = "Pipeline";

    inline static const Container::String VISUAL_TASK_LABEL_SUFFIX = " (Task)";
    inline static const Container::String VISUAL_CHECKPOINT_LABEL_SUFFIX = " (Checkpoint)";

    inline static const Container::String VISUAL_READ_ACCESS_COLOR = "#0000FFFF";
    inline static const Container::String VISUAL_WRITE_ACCESS_COLOR = "#FF0000FF";

    TaskRegister () = default;

    /// Copying task registers is counter-intuitive.
    TaskRegister (const TaskRegister &_other) = delete;

    TaskRegister (TaskRegister &&_other) = default;

    ~TaskRegister () = default;

    /// \brief Registers given task.
    void RegisterTask (Task _task) noexcept;

    /// \brief Registers new checkpoint with given name.
    /// \invariant Checkpoint name should be unique among checkpoints and tasks.
    void RegisterCheckpoint (Memory::UniqueString _name) noexcept;

    /// \brief Registers new resource with given name.
    /// \invariant Resource name should be unique.
    void RegisterResource (Memory::UniqueString _name) noexcept;

    /// \brief Exports registered tasks, checkpoints and resources as visual graph.
    /// \param _exportResources Should resources be exported?
    [[nodiscard]] VisualGraph::Graph ExportVisual (bool _exportResources) const noexcept;

    /// \brief Verifies registered data and exports it as task collection.
    /// \return Valid collection or empty collection if verification failed.
    /// \details Verifier checks that there is:
    ///          - No missing dependencies and resources.
    ///          - No circular dependencies.
    ///          - No data access races.
    [[nodiscard]] Emergence::Task::Collection ExportCollection () const noexcept;

    /// \brief Remove all registered tasks, checkpoints and resources.
    void Clear () noexcept;

    /// Assigning task registers is counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (TaskRegister);

private:
    friend class TaskGraph;

    void AssertNodeNameUniqueness (Memory::UniqueString _name) const noexcept;

    Container::Vector<Task> tasks {GetDefaultAllocationGroup ()};
    Container::Vector<Memory::UniqueString> checkpoints {GetDefaultAllocationGroup ()};
    Container::Vector<Memory::UniqueString> resources {GetDefaultAllocationGroup ()};
};
} // namespace Emergence::Flow
