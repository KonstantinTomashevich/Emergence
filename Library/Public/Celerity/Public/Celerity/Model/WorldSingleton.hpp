#pragma once

#include <atomic>
#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Defines how time is processed and how normal and fixed pipelines are executed.
enum class WorldUpdateMode
{
    /// \brief Default update mode.
    /// \details - Time is accumulated with no additional scale.
    ///          - Normal pipeline is executed each update.
    ///          - Time "produced" by normal pipeline is used to "feed" and execute fixed pipeline.
    SIMULATING = 0u,

    /// \brief Update mode for loading and pause screens.
    /// \details - Time is completely frozen.
    ///          - Normal pipeline is executed each frame with zero time step.
    ///          - Fixed pipeline is executed each frame with zero time step to compensate for changes made by
    ///            normal and custom, for example loading, pipelines. Fixed pipeline should not advance simulation
    ///            in any way.
    FROZEN
};

/// \brief Singleton for world<->tasks communication and global utility like id generation.
/// \warning This singleton is modified by World outside of pipeline execution,
///          therefore OnChange events do not work with it.
struct WorldSingleton final
{
    /// \brief Indicates whether current normal update was separated from previous one by one or more fixed updates.
    /// \warning Access outside of normal update routine leads to undefined behaviour.
    bool fixedUpdateHappened = false;

    /// \brief Current update mode for the game world.
    WorldUpdateMode updateMode = WorldUpdateMode::SIMULATING;

    /// \invariant Do not access directly, use ::GenerateId.
    std::atomic_uintptr_t idCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    std::uintptr_t GenerateId () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId fixedUpdateHappened;
        StandardLayout::FieldId updateMode;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

// TODO: We need to split Celerity and Celerity extensions into Model and Logic parts.
//       For example, tools need model all the time, but almost never need logic.
} // namespace Emergence::Celerity
