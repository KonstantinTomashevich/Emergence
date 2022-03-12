#pragma once

#include <Celerity/Event/EventTrigger.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/Query/ModifySingletonQuery.hpp>

#include <Container/TypedOrderedPool.hpp>

#include <Memory/OrderedPool.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Celerity
{
struct TimeSingleton;
struct WorldSingleton;

class World final
{
public:
    explicit World (Memory::UniqueString _name) noexcept;

    World (const World &_other) = delete;

    World (World &&_other) = delete;

    ~World ();

    /// \brief Executes normal update and fixed update if needed.
    void Update () noexcept;

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

    void NormalUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    void FixedUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    Pipeline *AddPipeline (Memory::UniqueString _id,
                           PipelineType _type,
                           const Task::Collection &_collection,
                           std::size_t _maximumChildThreads) noexcept;

    Warehouse::Registry registry;

    Warehouse::ModifySingletonQuery modifyTime;
    Warehouse::ModifySingletonQuery modifyWorld;

    Memory::OrderedPool pipelinePool;
    Pipeline *normalPipeline = nullptr;
    Pipeline *fixedPipeline = nullptr;

    std::array<EventScheme, static_cast<std::size_t> (PipelineType::COUNT)> eventSchemes;
};

class WorldTestingUtility final
{
public:
    WorldTestingUtility () = delete;

    static void RunNormalUpdateOnce (World &_world, uint64_t _timeDeltaNs) noexcept;

    static void RunFixedUpdateOnce (World &_world) noexcept;

private:
    static std::pair<TimeSingleton *, WorldSingleton *> ExtractSingletons (World &_world) noexcept;
};
} // namespace Emergence::Celerity
