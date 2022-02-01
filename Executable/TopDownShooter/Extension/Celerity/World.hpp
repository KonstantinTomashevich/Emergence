#pragma once

#include <Celerity/Pipeline.hpp>

#include <Container/Vector.hpp>

#include <Memory/UnorderedPool.hpp>

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

    // TODO: Rules for automatic edition/addition/deletion event firing.

private:
    friend class PipelineBuilder;
    friend class TaskConstructor;
    friend class WorldTestingUtility;

    struct PipelineNode final
    {
        Pipeline pipeline;
        PipelineNode *next;
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

    Memory::UnorderedPool pipelinePool;
    PipelineNode *firstPipeline = nullptr;
    PipelineNode *normalPipeline = nullptr;
    PipelineNode *fixedPipeline = nullptr;
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
