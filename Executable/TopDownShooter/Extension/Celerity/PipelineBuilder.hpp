#pragma once

#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Container/HashSet.hpp>
#include <Container/Vector.hpp>

#include <Flow/TaskRegister.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
class TaskConstructor final
{
public:
    TaskConstructor (const TaskConstructor &_other) = delete;

    TaskConstructor (TaskConstructor &&_other) noexcept;

    ~TaskConstructor () noexcept;

    void DependOn (Memory::UniqueString _taskOrCheckpoint) noexcept;

    void MakeDependencyOf (Memory::UniqueString _taskOrCheckpoint) noexcept;

    [[nodiscard]] Warehouse::FetchSingletonQuery FetchSingleton (const StandardLayout::Mapping &_typeMapping);

    [[nodiscard]] Warehouse::ModifySingletonQuery ModifySingleton (
        const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::InsertShortTermQuery InsertShortTerm (
        const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::FetchSequenceQuery FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::ModifySequenceQuery ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::InsertLongTermQuery InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::FetchValueQuery FetchValue (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] Warehouse::ModifyValueQuery ModifyValue (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] Warehouse::FetchAscendingRangeQuery FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                           StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::ModifyAscendingRangeQuery ModifyAscendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::FetchDescendingRangeQuery FetchDescendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::ModifyDescendingRangeQuery ModifyDescendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::FetchShapeIntersectionQuery FetchShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::FetchRayIntersectionQuery FetchRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::ModifyRayIntersectionQuery ModifyRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    void SetExecutor (std::function<void ()> _executor) noexcept;

    template <typename Executor, typename... Args>
    void SetExecutor (Args... _args) noexcept;

    [[nodiscard]] World *GetWorld () const noexcept;

    TaskConstructor &operator= (const TaskConstructor &_other) = delete;

    /// Move-assignment is allowed, because it makes construction of several tasks from one function easier.
    TaskConstructor &operator= (TaskConstructor &&_other) noexcept;

private:
    friend class PipelineBuilder;

    TaskConstructor (PipelineBuilder *_parent, Memory::UniqueString _name) noexcept;

    PipelineBuilder *parent;
    Flow::Task task;
    Memory::Heap heap;
};

class PipelineBuilder final
{
public:
    explicit PipelineBuilder (World *_targetWorld) noexcept;

    PipelineBuilder (const PipelineBuilder &_other) = delete;

    PipelineBuilder (PipelineBuilder &&_other) = delete;

    ~PipelineBuilder () = default;

    bool Begin (Memory::UniqueString _id, PipelineType _type) noexcept;

    [[nodiscard]] TaskConstructor AddTask (Memory::UniqueString _name) noexcept;

    void AddCheckpoint (Memory::UniqueString _name) noexcept;

    [[nodiscard]] Pipeline *End (std::size_t _maximumChildThreads) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PipelineBuilder);

private:
    friend class TaskConstructor;

    void FinishTaskRegistration (Flow::Task _task) noexcept;

    World *world;
    Memory::UniqueString currentPipelineId;
    PipelineType currentPipelineType;
    Memory::Profiler::AllocationGroup currentPipelineAllocationGroup;

    Flow::TaskRegister taskRegister;
    Container::HashSet<Memory::UniqueString> registeredResources;
};

template <typename Successor>
class TaskExecutorBase : public Handling::HandleableBase
{
public:
    TaskExecutorBase () = default;

    TaskExecutorBase (const TaskExecutorBase &_other) = delete;

    TaskExecutorBase (TaskExecutorBase &&_other) = delete;

    ~TaskExecutorBase () noexcept
    {
        static_assert (std::is_base_of_v<TaskExecutorBase<Successor>, Successor>);
        heap.Release (static_cast<Successor *> (this), sizeof (Successor));
    }

    void LastReferenceUnregistered () noexcept
    {
        static_cast<Successor *> (this)->~Successor ();
    }

    EMERGENCE_DELETE_ASSIGNMENT (TaskExecutorBase);

protected:
    Memory::Heap heap {Memory::Profiler::AllocationGroup::Top ()};
};

template <typename Executor, typename... Args>
void TaskConstructor::SetExecutor (Args... _args) noexcept
{
    static_assert (std::is_base_of_v<TaskExecutorBase<Executor>, Executor>);
    auto placeholder = heap.GetAllocationGroup ().PlaceOnTop ();
    Handling::Handle<Executor> handle {new (heap.Acquire (sizeof (Executor)))
                                           Executor {*this, std::forward<Args> (_args)...}};

    SetExecutor (
        [handle] ()
        {
            handle->Execute ();
        });
}
} // namespace Emergence::Celerity
