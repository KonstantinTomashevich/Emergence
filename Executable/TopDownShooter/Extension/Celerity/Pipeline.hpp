#pragma once

#include <Memory/UniqueString.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
class Pipeline final
{
public:
    Pipeline (const Pipeline &_other) = delete;

    Pipeline (Pipeline &&_other) = delete;

    ~Pipeline () = default;

    Memory::UniqueString GetId () const noexcept;

    void Execute () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Pipeline);

private:
    friend class World;

    explicit Pipeline (Memory::UniqueString _id, const Task::Collection &_collection, std::size_t _maximumChildThreads);

    Memory::UniqueString id;
    Task::Executor executor;
};
} // namespace Emergence::Celerity
