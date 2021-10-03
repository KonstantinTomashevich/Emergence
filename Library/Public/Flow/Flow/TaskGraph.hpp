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

class TaskGraph final
{
public:
    TaskGraph () = default;

    /// Copying task graphs is counter-intuitive.
    TaskGraph (const TaskGraph &_other) = delete;

    TaskGraph (TaskGraph &&_other) = default;

    ~TaskGraph () = default;

    void AddTask (Task _task) noexcept;

    void RegisterCheckpoint (const char *_name) noexcept;

    void RegisterResource (const char *_name) noexcept;

    [[nodiscard]] VisualGraph::Graph ExportVisual (bool _exportResources) const noexcept;

    [[nodiscard]] TaskCollection ExportCollection () const noexcept;

    /// Assigning task graphs is counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (TaskGraph);

private:
    void AssertNodeNameUniqueness (const char *_name) const noexcept;

    bool BuildGraph (struct Graph *_graph) const noexcept;

    std::vector<Task> tasks;
    std::vector<std::string> checkpoints;
    std::vector<std::string> resources;
};
} // namespace Emergence::Flow
