#pragma once

#include <Celerity/Event/EventTrigger.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/Query/ModifySingletonQuery.hpp>

#include <Container/TypedOrderedPool.hpp>

#include <Memory/OrderedPool.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Celerity
{
struct WorldSingleton;

// TODO: Note about performance.
//       Celerity is built on top of Warehouse and RecordCollection, and that services use standard layout mappings
//       extensively. It is convenient from code architecture perspective, but may result in performance problems.
//       There is a lot of mapping-related logic, that should be optimized, but nevertheless mapping usage creates
//       unavoidable performance barrier.
//       If all modules (both engine-level and game-level) are compiled from single source, then all used mapping data
//       is visible during compile time. We can use this fact for Chameleon-like approach: scan codebase and generate
//       optimized code for all storages, that supports all use cases by hard coding them. To make code scanning easier
//       we might use macro-driven approach like UnrealEngine. This code generation allows us to achieve hard code
//       performance without losing Celerity features, but it has one significant downside: we can no longer separate
//       game source and engine source, otherwise Chameleon will not be able to generate code for all use cases.

/// \brief Contains basic configuration for WorldSingleton and TimeSingleton.
/// \details These values can be set through initialization pipeline, therefore
///          this structure is only a more convenient wa to do this task.
struct WorldConfiguration final
{
    /// \see TimeSingleton::targetFixedFrameDurationsS
    Container::InplaceVector<float, TimeSingleton::MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS {
        1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f};
};

/// \brief Represents whole game level (or world itself), works as conduit for data, events and pipelines.
class World final
{
public:
    World (Memory::UniqueString _name, const WorldConfiguration &_configuration = {}) noexcept;

    World (const World &_other) = delete;

    World (World &&_other) = delete;

    ~World ();

    /// \brief Executes normal update and fixed update if needed.
    /// \details Calculates time deltas and updates time, then executes normal pipeline and decides whether it is
    ///          needed to execute fixed pipeline.
    /// \invariant Should be called every frame if gameplay world is active. Correctly processes long absence of calls.
    void Update () noexcept;

    /// \brief Removes given pipeline from the world and frees used memory.
    /// \invariant Pipeline belongs to this world.
    void RemovePipeline (Pipeline *_pipeline) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (World);

private:
    friend class EventRegistrar;
    friend class PipelineBuilder;
    friend class TaskConstructor;
    friend class WorldTestingUtility;

    struct CustomEventInfo final
    {
        StandardLayout::Mapping type;
        EventRoute route;
    };

    struct EventScheme final
    {
        EventScheme (const Memory::Profiler::AllocationGroup &_rootAllocationGroup) noexcept;

        Container::TypedOrderedPool<CustomEventInfo> custom;
        Container::TypedOrderedPool<TrivialEventTriggerRow> onAdd;
        Container::TypedOrderedPool<TrivialEventTriggerRow> onRemove;
        Container::TypedOrderedPool<OnChangeEventTrigger> onChange;
        Container::TypedOrderedPool<ChangeTracker> changeTrackers;
    };

    static void TimeUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    void FixedUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    Pipeline *AddPipeline (Memory::UniqueString _id, PipelineType _type, const Task::Collection &_collection) noexcept;

    Warehouse::Registry registry;

    Warehouse::ModifySingletonQuery modifyTime;
    Warehouse::ModifySingletonQuery modifyWorld;

    Memory::OrderedPool pipelinePool;
    Pipeline *normalPipeline = nullptr;
    Pipeline *fixedPipeline = nullptr;

    std::array<EventScheme, static_cast<std::size_t> (PipelineType::COUNT)> eventSchemes;
};

/// \brief Contains useful functions for tests.
class WorldTestingUtility final
{
public:
    WorldTestingUtility () = delete;

    /// \brief Updates only normal time by given delta and runs normal update once.
    /// \warning Do not use if normal-fixed time integrity is required! Update world conventionally instead.
    static void RunNormalUpdateOnce (World &_world, uint64_t _timeDeltaNs) noexcept;

    /// \brief Updates only fixed time by one frame and runs fixed update once.
    /// \warning Do not use if normal-fixed time integrity is required! Update world conventionally instead.
    static void RunFixedUpdateOnce (World &_world) noexcept;

private:
    static std::pair<TimeSingleton *, WorldSingleton *> ExtractSingletons (World &_world) noexcept;
};
} // namespace Emergence::Celerity
