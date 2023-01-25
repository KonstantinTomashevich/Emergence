#pragma once

#include <Memory/UniqueString.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
/// \brief Pipeline type is used to decide how to treat pipeline execution in World context.
enum class PipelineType
{
    /// \brief Normal pipelines contain visual logic and are executed every frame.
    /// \details There could be only one normal pipeline per World.
    NORMAL = 0u,

    /// \brief Fixed pipelines contain "strong" logic (physics, gameplay, network, etc)
    ///        and are executed with fixed time delta.
    /// \details There could be only one fixed pipeline per World.
    FIXED,

    /// \brief Custom pipelines are created for special purposes and are executed directly by user.
    CUSTOM,

    /// \brief Helper value for counting pipeline types.
    COUNT
};

/// \return Human readable pipeline type value.
const char *GetPipelineTypeName (PipelineType _type);

/// \brief Executable collection of interdependent tasks.
/// \details To create pipelines for World, use PipelineBuilder.
class Pipeline final
{
public:
    Pipeline (const Pipeline &_other) = delete;

    Pipeline (Pipeline &&_other) = delete;

    ~Pipeline () = default;

    /// \return Human readable pipeline id.
    [[nodiscard]] Memory::UniqueString GetId () const noexcept;

    [[nodiscard]] PipelineType GetType () const noexcept;

    /// \brief Executes all tasks in pipeline and returns after all tasks are executed.
    void Execute () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Pipeline);

private:
    friend class WorldView;

    explicit Pipeline (Memory::UniqueString _id, PipelineType _type, const Task::Collection &_collection);

    Memory::UniqueString id;
    PipelineType type;

    const Memory::UniqueString beginMarker;
    const Memory::UniqueString endMarker;
    Task::Executor executor;
};
} // namespace Emergence::Celerity
