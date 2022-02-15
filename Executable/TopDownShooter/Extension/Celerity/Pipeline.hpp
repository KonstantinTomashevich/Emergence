#pragma once

#include <Memory/UniqueString.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
enum class PipelineType
{
    NORMAL = 0u,
    FIXED,
    CUSTOM,
    COUNT
};

class Pipeline final
{
public:
    Pipeline (const Pipeline &_other) = delete;

    Pipeline (Pipeline &&_other) = delete;

    ~Pipeline () = default;

    [[nodiscard]] Memory::UniqueString GetId () const noexcept;

    [[nodiscard]] PipelineType GetType () const noexcept;

    void Execute () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Pipeline);

private:
    friend class World;

    explicit Pipeline (Memory::UniqueString _id,
                       PipelineType _type,
                       const Task::Collection &_collection,
                       std::size_t _maximumChildThreads);

    Memory::UniqueString id;
    PipelineType type;

    const Memory::UniqueString beginMarker;
    const Memory::UniqueString endMarker;
    Task::Executor executor;
};
} // namespace Emergence::Celerity
