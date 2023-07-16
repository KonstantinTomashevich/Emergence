#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Task/Collection.hpp>

namespace Emergence::Task
{
/// \brief Executes tasks from source collection using implementation-specific mechanism.
class Executor final
{
public:
    /// \brief Constructs executor for given task collection.
    /// \invariant There is no circular dependencies in given task collection.
    explicit Executor (const Collection &_collection) noexcept;

    /// Copying executors is counter-intuitive.
    Executor (const Executor &_other) = delete;

    Executor (Executor &&_other) noexcept;

    ~Executor () noexcept;

    /// \brief Starts task execution and returns only when all tasks are finished.
    /// \details Implementations may use their own threads to execute tasks.
    void Execute () noexcept;

    /// Assigning executors is counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Executor);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);
};
} // namespace Emergence::Task
