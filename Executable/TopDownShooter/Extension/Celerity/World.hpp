#pragma once

#include <atomic>

#include <Celerity/Pipeline.hpp>

#include <Container/Vector.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Celerity
{
class World final
{
public:
    explicit World (Memory::UniqueString _name) noexcept;

    World (const World &_other) = delete;

    World (World &&_other) = delete;

    ~World ();

    std::uintptr_t GetNextObjectId () noexcept;

    Warehouse::FetchSingletonQuery FetchSingletonExternally (const StandardLayout::Mapping &_mapping) noexcept;

    Warehouse::ModifySingletonQuery ModifySingletonExternally (const StandardLayout::Mapping &_mapping) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (World);

    // TODO: Rules for automatic edition/addition/deletion event firing.

private:
    friend class PipelineBuilder;
    friend class TaskConstructor;

    Pipeline *AddPipeline (Memory::UniqueString _id, const Task::Collection &_collection, std::size_t _maximumChildThreads);

    Warehouse::Registry registry;
    Memory::Heap pipelineHeap;
    Container::Vector<Pipeline *> pipelines;
    std::atomic_unsigned_lock_free objectIdCounter;
};
} // namespace Emergence::Celerity
