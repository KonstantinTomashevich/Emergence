#pragma once

#include <functional>
#include <string>
#include <vector>

#include <API/Common/Shortcuts.hpp>

#include <Task/Collection.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Flow
{
/// \brief Contains all useful information about tasks.
struct Task
{
    /// \brief Task name is used for dependency connection, logging and debugging.
    /// \invariant Must be unique among tasks and checkpoints.
    std::string name;

    /// \see TaskCollection::Item::task
    std::function<void ()> executor;

    /// \brief Names of resources, that are read by this task.
    std::vector<std::string> readAccess;

    /// \brief Names of resources, that are modified by this task.
    /// \invariant If task both reads and modifies one resources, this resource
    ///            should be added to ::writeAccess, but not to ::readAccess.
    std::vector<std::string> writeAccess;

    /// \brief Names of tasks, on which this task depends.
    std::vector<std::string> dependsOn;

    /// \brief Names of tasks, to which this task injects itself as dependency.
    std::vector<std::string> dependencyOf;
};

/// \brief Allows user to register tasks and export result as task collection or visual graph.
/// \details Registration order doesn't matter: all verification will be done during ::ExportCollection.
class TaskRegister final
{
public:
    inline static const std::string VISUAL_ROOT_GRAPH_ID = "TaskGraph";
    inline static const std::string VISUAL_RESOURCE_GRAPH_ID = "Resources";
    inline static const std::string VISUAL_PIPELINE_GRAPH_ID = "Pipeline";

    inline static const std::string VISUAL_TASK_LABEL_SUFFIX = " (Task)";
    inline static const std::string VISUAL_CHECKPOINT_LABEL_SUFFIX = " (Checkpoint)";

    inline static const std::string VISUAL_READ_ACCESS_COLOR = "#0000FFFF";
    inline static const std::string VISUAL_WRITE_ACCESS_COLOR = "#FF0000FF";

    TaskRegister () = default;

    /// Copying task registers is counter-intuitive.
    TaskRegister (const TaskRegister &_other) = delete;

    TaskRegister (TaskRegister &&_other) = default;

    ~TaskRegister () = default;

    /// \brief Registers given task.
    void RegisterTask (Task _task) noexcept;

    /// \brief Registers new checkpoint with given name.
    /// \invariant Checkpoint name should be unique among checkpoints and tasks.
    void RegisterCheckpoint (const char *_name) noexcept;

    /// \brief Registers new resources with given name.
    /// \invariant Resource name should be unique.
    void RegisterResource (const char *_name) noexcept;

    /// \brief Exports registered tasks, checkpoints and resources as visual graph.
    /// \param _exportResources Should resources be exported?
    [[nodiscard]] VisualGraph::Graph ExportVisual (bool _exportResources) const noexcept;

    /// \brief Verifies registered data and exports it as task collection.
    /// \return Valid collection or empty collection if verification failed.
    /// \details Verifier checks that there is:
    ///          - No missing dependencies and resources.
    ///          - No dependency cycles.
    ///          - No data access races.
    [[nodiscard]] TaskCollection ExportCollection () const noexcept;

    /// Assigning task registers is counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (TaskRegister);

private:
    void AssertNodeNameUniqueness (const char *_name) const noexcept;

    bool BuildGraph (struct Graph *_graph) const noexcept;

    std::vector<Task> tasks;
    std::vector<std::string> checkpoints;
    std::vector<std::string> resources;
};
} // namespace Emergence::Flow
