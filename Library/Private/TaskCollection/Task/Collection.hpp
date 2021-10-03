#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Emergence
{
/// \brief Intermediate structure for storing tasks that may depend on each other.
/// \details This format is designed for transporting information about tasks from one library to another.
///          It's not designed to be modified after initial construction and setup.
struct TaskCollection
{
    /// \brief Contains info about one tasks.
    struct Item
    {
        /// \brief Task name, can be used for debugging and logging.
        std::string name;

        /// \brief Function, that starts task execution and returns only when task execution is finished.
        std::function<void ()> task;

        /// \brief Indices of ::tasks on which this task depends.
        std::vector<std::size_t> dependencyIndices;
    };

    /// \brief Contains information about all tasks in this collection.
    std::vector<Item> tasks;
};
} // namespace Emergence
