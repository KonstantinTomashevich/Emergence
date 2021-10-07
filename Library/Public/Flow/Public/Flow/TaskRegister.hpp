#pragma once

#include <functional>
#include <string>
#include <vector>

#include <API/Common/Shortcuts.hpp>

#include <Task/Collection.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Flow
{
struct Task
{
    std::string name;
    std::function<void ()> executor;

    std::vector<std::string> readAccess;
    std::vector<std::string> writeAccess;

    std::vector<std::string> dependsOn;
    std::vector<std::string> dependencyOf;
};

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

    void RegisterTask (Task _task) noexcept;

    void RegisterCheckpoint (const char *_name) noexcept;

    void RegisterResource (const char *_name) noexcept;

    [[nodiscard]] VisualGraph::Graph ExportVisual (bool _exportResources) const noexcept;

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
