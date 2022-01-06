#pragma once

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
class Pipeline final
{
public:
    Pipeline (const Pipeline &_other) = delete;

    Pipeline (Pipeline &&_other) = delete;

    ~Pipeline () = default;

    void Execute () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Pipeline);

private:
    friend class World;

    explicit Pipeline (const Task::Collection &_collection, std::size_t _maximumChildThreads);

    Task::Executor executor;
};
} // namespace Emergence::Celerity
