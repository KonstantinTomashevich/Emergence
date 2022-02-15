#pragma once

#include <Celerity/Pipeline.hpp>
#include <Celerity/Query/FetchAscendingRangeQuery.hpp>
#include <Celerity/Query/FetchDescendingRangeQuery.hpp>
#include <Celerity/Query/FetchRayIntersectionQuery.hpp>
#include <Celerity/Query/FetchSequenceQuery.hpp>
#include <Celerity/Query/FetchShapeIntersectionQuery.hpp>
#include <Celerity/Query/FetchSingletonQuery.hpp>
#include <Celerity/Query/FetchValueQuery.hpp>
#include <Celerity/Query/InsertLongTermQuery.hpp>
#include <Celerity/Query/InsertShortTermQuery.hpp>
#include <Celerity/Query/ModifyAscendingRangeQuery.hpp>
#include <Celerity/Query/ModifyDescendingRangeQuery.hpp>
#include <Celerity/Query/ModifyRayIntersectionQuery.hpp>
#include <Celerity/Query/ModifySequenceQuery.hpp>
#include <Celerity/Query/ModifyShapeIntersectionQuery.hpp>
#include <Celerity/Query/ModifySingletonQuery.hpp>
#include <Celerity/Query/ModifyValueQuery.hpp>
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

    [[nodiscard]] FetchSingletonQuery FetchSingleton (const StandardLayout::Mapping &_typeMapping);

    [[nodiscard]] ModifySingletonQuery ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] InsertShortTermQuery InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] FetchSequenceQuery FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] ModifySequenceQuery ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] InsertLongTermQuery InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] FetchValueQuery FetchValue (const StandardLayout::Mapping &_typeMapping,
                                              const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] ModifyValueQuery ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                                const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] FetchAscendingRangeQuery FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] ModifyAscendingRangeQuery ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                  StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] FetchDescendingRangeQuery FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                  StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] ModifyDescendingRangeQuery ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                    StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] FetchShapeIntersectionQuery FetchShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] FetchRayIntersectionQuery FetchRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] ModifyRayIntersectionQuery ModifyRayIntersection (
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

    [[nodiscard]] TrivialEventTriggerRow *FindEventsOnRemove (
        const StandardLayout::Mapping &_trackedType) const noexcept;

    [[nodiscard]] ChangeTracker *FindChangeTracker (const StandardLayout::Mapping &_trackedType) const noexcept;

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

    Pipeline *End (std::size_t _maximumChildThreads) noexcept;

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
