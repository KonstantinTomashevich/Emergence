#pragma once

#include <Celerity/Pipeline.hpp>
#include <Celerity/Query/EditAscendingRangeQuery.hpp>
#include <Celerity/Query/EditDescendingRangeQuery.hpp>
#include <Celerity/Query/EditRayIntersectionQuery.hpp>
#include <Celerity/Query/EditShapeIntersectionQuery.hpp>
#include <Celerity/Query/EditSignalQuery.hpp>
#include <Celerity/Query/EditValueQuery.hpp>
#include <Celerity/Query/FetchAscendingRangeQuery.hpp>
#include <Celerity/Query/FetchDescendingRangeQuery.hpp>
#include <Celerity/Query/FetchRayIntersectionQuery.hpp>
#include <Celerity/Query/FetchSequenceQuery.hpp>
#include <Celerity/Query/FetchShapeIntersectionQuery.hpp>
#include <Celerity/Query/FetchSignalQuery.hpp>
#include <Celerity/Query/FetchSingletonQuery.hpp>
#include <Celerity/Query/FetchValueQuery.hpp>
#include <Celerity/Query/InsertLongTermQuery.hpp>
#include <Celerity/Query/InsertShortTermQuery.hpp>
#include <Celerity/Query/ModifyAscendingRangeQuery.hpp>
#include <Celerity/Query/ModifyDescendingRangeQuery.hpp>
#include <Celerity/Query/ModifyRayIntersectionQuery.hpp>
#include <Celerity/Query/ModifySequenceQuery.hpp>
#include <Celerity/Query/ModifyShapeIntersectionQuery.hpp>
#include <Celerity/Query/ModifySignalQuery.hpp>
#include <Celerity/Query/ModifySingletonQuery.hpp>
#include <Celerity/Query/ModifyValueQuery.hpp>
#include <Celerity/Query/RemoveAscendingRangeQuery.hpp>
#include <Celerity/Query/RemoveDescendingRangeQuery.hpp>
#include <Celerity/Query/RemoveRayIntersectionQuery.hpp>
#include <Celerity/Query/RemoveShapeIntersectionQuery.hpp>
#include <Celerity/Query/RemoveSignalQuery.hpp>
#include <Celerity/Query/RemoveValueQuery.hpp>
#include <Celerity/World.hpp>

#include <Container/HashSet.hpp>
#include <Container/Vector.hpp>

#include <Flow/TaskRegister.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
/// \brief RAII-style constructor for Pipeline tasks.
/// \note We have 4 access types for long term objects because of automated event triggering logic. If task has
///       edit access, it is registered as OnChange event producer, because it is able to change objects. If task
///       has remove access, it is registered as OnRemove event producer for the same reason. This makes it difficult
///       to correctly organize event pipelines as we have a lot of potential producers. To reduce this side effect
///       we allow users to request edit access without remove and remove without edit.
class TaskConstructor final
{
public:
    TaskConstructor (const TaskConstructor &_other) = delete;

    TaskConstructor (TaskConstructor &&_other) noexcept;

    ~TaskConstructor () noexcept;

    /// \brief Registers dependency on another task or checkpoint with given name.
    /// \details There is no registration order requirement: given task or checkpoint is allowed to be registered later.
    TaskConstructor &DependOn (Memory::UniqueString _taskOrCheckpoint) noexcept;

    /// \brief Registers this task as dependency of another task or checkpoint with given name.
    /// \details There is no registration order requirement: given task or checkpoint is allowed to be registered later.
    TaskConstructor &MakeDependencyOf (Memory::UniqueString _taskOrCheckpoint) noexcept;

    /// \brief Grants read-only access to singleton, described by given mapping.
    [[nodiscard]] FetchSingletonQuery FetchSingleton (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants read-write access to singleton, described by given mapping.
    [[nodiscard]] ModifySingletonQuery ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants insertion access to short term objects storage, described by given mapping.
    [[nodiscard]] InsertShortTermQuery InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants read-only access to short term objects storage, described by given mapping.
    [[nodiscard]] FetchSequenceQuery FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants read-write access to short term objects storage, described by given mapping.
    [[nodiscard]] ModifySequenceQuery ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants insertion access to long term objects storage, described by given mapping.
    [[nodiscard]] InsertLongTermQuery InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with any selected values in given fields.
    [[nodiscard]] FetchValueQuery FetchValue (const StandardLayout::Mapping &_typeMapping,
                                              const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with any selected values in given fields.
    [[nodiscard]] ModifyValueQuery ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                                const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with any selected values in given fields.
    [[nodiscard]] EditValueQuery EditValue (const StandardLayout::Mapping &_typeMapping,
                                            const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with any selected values in given fields.
    [[nodiscard]] RemoveValueQuery RemoveValue (const StandardLayout::Mapping &_typeMapping,
                                                const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] FetchAscendingRangeQuery FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] ModifyAscendingRangeQuery ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                  StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] EditAscendingRangeQuery EditAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                              StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] RemoveAscendingRangeQuery RemoveAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                  StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] FetchDescendingRangeQuery FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                  StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] ModifyDescendingRangeQuery ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                    StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] EditDescendingRangeQuery EditDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects within selected value interval on given field.
    [[nodiscard]] RemoveDescendingRangeQuery RemoveDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                    StandardLayout::FieldId _keyField) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with given value in given field.
    [[nodiscard]] FetchSignalQuery FetchSignal (const StandardLayout::Mapping &_typeMapping,
                                                StandardLayout::FieldId _keyField,
                                                const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with given value in given field.
    [[nodiscard]] ModifySignalQuery ModifySignal (
        const StandardLayout::Mapping &_typeMapping,
        StandardLayout::FieldId _keyField,
        const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with given value in given field.
    [[nodiscard]] EditSignalQuery EditSignal (const StandardLayout::Mapping &_typeMapping,
                                              StandardLayout::FieldId _keyField,
                                              const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects with given value in given field.
    [[nodiscard]] RemoveSignalQuery RemoveSignal (
        const StandardLayout::Mapping &_typeMapping,
        StandardLayout::FieldId _keyField,
        const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected shape.
    [[nodiscard]] FetchShapeIntersectionQuery FetchShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected shape.
    [[nodiscard]] ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected shape.
    [[nodiscard]] EditShapeIntersectionQuery EditShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected shape.
    [[nodiscard]] RemoveShapeIntersectionQuery RemoveShapeIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants read-only access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected ray.
    [[nodiscard]] FetchRayIntersectionQuery FetchRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants edit+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected ray.
    [[nodiscard]] ModifyRayIntersectionQuery ModifyRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants edit access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected ray.
    [[nodiscard]] EditRayIntersectionQuery EditRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Grants read+remove access to long term objects storage, described by given mapping, through
    ///        prepared query, that allows iterating over objects that intersect with selected ray.
    [[nodiscard]] RemoveRayIntersectionQuery RemoveRayIntersection (
        const StandardLayout::Mapping &_typeMapping,
        const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept;

    /// \brief Make given lambda task executor.
    TaskConstructor &SetExecutor (std::function<void ()> _executor) noexcept;

    /// \brief Make instance of given class, that inherits TaskExecutorBase, task executor.
    /// \details Technically it is just more convenient way to set executor that lambda variant.
    ///          Under the hood it still calls constructor and creates lambda, while also taking care
    ///          memory usage profiler grouping.
    /// \invariant Constructor of `Executor` has `Executor (TaskConstructor&, Args...) noexcept` signature.
    template <typename Executor, typename... Args>
    TaskConstructor &SetExecutor (Args... _args) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TaskConstructor);

private:
    friend class PipelineBuilder;

    TaskConstructor (PipelineBuilder *_parent, Memory::UniqueString _name) noexcept;

    void RegisterReadAccess (Memory::UniqueString _resourceName) noexcept;

    void RegisterWriteAccess (Memory::UniqueString _resourceName) noexcept;

    [[nodiscard]] TrivialEventTriggerRow *BindTrivialEvents (Container::TypedOrderedPool<TrivialEventTriggerRow> &_rows,
                                                             const StandardLayout::Mapping &_trackedType) noexcept;

    [[nodiscard]] TrivialEventTriggerRow *BindEventsOnAdd (const StandardLayout::Mapping &_trackedType) noexcept;

    [[nodiscard]] TrivialEventTriggerRow *BindEventsOnRemove (const StandardLayout::Mapping &_trackedType) noexcept;

    [[nodiscard]] ChangeTracker *BindChangeTracker (const StandardLayout::Mapping &_trackedType) noexcept;

    void RegisterEventProduction (const StandardLayout::Mapping &_eventType) noexcept;

    void RegisterEventConsumption (const StandardLayout::Mapping &_eventType) noexcept;

    PipelineBuilder *parent;
    Flow::Task task;
    Memory::Heap heap;
};

/// \brief Provides API for building WorldView Pipeline's.
class PipelineBuilder final
{
public:
    explicit PipelineBuilder (WorldView *_targetWorldView) noexcept;

    PipelineBuilder (const PipelineBuilder &_other) = delete;

    PipelineBuilder (PipelineBuilder &&_other) = delete;

    ~PipelineBuilder () = default;

    /// \brief Begin building new Pipeline for associated world view.
    /// \return Whether building routine was successfully started. Building routine fails when user tries to create
    ///         second normal or second fixed pipeline.
    bool Begin (Memory::UniqueString _id, PipelineType _type) noexcept;

    /// \brief Starts construction routine for new task.
    /// \invariant Task name is unique.
    /// \invariant It's forbidden to construct several tasks simultaneously.
    [[nodiscard]] TaskConstructor AddTask (Memory::UniqueString _name) noexcept;

    /// \brief Registers new checkpoint.
    /// \invariant Checkpoint name is unique.
    void AddCheckpoint (Memory::UniqueString _name) noexcept;

    /// \brief Calls TaskRegister::OpenVisualGroup for task visual grouping.
    Flow::TaskRegister::VisualGroupNodePlaced OpenVisualGroup (Container::String _name) noexcept;

    /// \brief Finishes active pipeline routine.
    /// \param _visualGraphOutput If pipeline can be created and this parameter is not `nullptr`,
    ///                           pipeline visual graph will be assigned to the given address.
    /// \return New pipeline or `nullptr` if pipeline creation fails due to errors.
    Pipeline *End (VisualGraph::Graph *_visualGraphOutput = nullptr, bool _exportResourcesToGraph = false) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PipelineBuilder);

private:
    friend class TaskConstructor;

    /// \brief For each event type, contains set of ids of tasks that use it in associated context.
    using EventUsageMap = Container::HashMap<StandardLayout::Mapping, Container::HashSet<Memory::UniqueString>>;

    Memory::Profiler::AllocationGroup GetBuildTimeAllocationGroup () noexcept;

    void ImportEventScheme (const World::EventScheme &_scheme) noexcept;

    void FinishTaskRegistration (Flow::Task _task) noexcept;

    /// \details In continuous routine, events from current execution can be processed by next execution.
    ///          Therefore, for each event type pipeline tasks can be separate into ordered category list:
    ///          PreviousExecutionConsumers -> ClearingTask -> Producers -> CurrentExecutionConsumers.
    ///
    ///          This method verifies that such separation can be done for each event type and adds clearing task.
    void PostProcessContinuousEventRoutine (const EventUsageMap &_production, const EventUsageMap &_consumption);

    /// \details In local event routine, events from current execution are visible only during this execution.
    ///          Therefore, for each event type pipeline tasks can be separate into ordered category list:
    ///          Producers -> Consumers -> ClearingTask.
    ///
    ///          This method verifies that such separation can be done for each event type and adds clearing task.
    void PostProcessLocalEventRoutine (const EventUsageMap &_production, const EventUsageMap &_consumption);

    /// \details Adds clearing task for all ::sharedEventTypes, that are consumed in current pipeline.
    void PostProcessSharedEventRoutine (const EventUsageMap &_consumption);

    WorldView *worldView;
    Memory::UniqueString currentPipelineId;
    PipelineType currentPipelineType;
    Memory::Profiler::AllocationGroup currentPipelineAllocationGroup;

    Flow::TaskRegister taskRegister;
    Container::HashSet<Memory::UniqueString> registeredResources;

    /// \details Event cleaners need ModifySequenceQuery's to delete events, but creating these queries for
    ///          events is generally forbidden. Therefore we add this flag to temporary allow query creation.
    bool postProcessingEvents = false;

    /// \brief If there is any build-time errors, pipeline will not be created in ::End.
    bool anyErrorsDetected = false;

    // TODO: Maps/sets bellow should be replaced with their flat alternatives.

    /// \brief Used to check whether given type is event type or not.
    Container::HashSet<StandardLayout::Mapping> eventTypes;

    /// \brief Types of events, that are produced in one pipeline and consumed in another.
    /// \details For example, see EventRoute::FROM_FIXED_TO_NORMAL.
    Container::HashSet<StandardLayout::Mapping> sharedEventTypes;

    /// \brief Types of events, that are produced automatically by on add, on remove and on change triggers.
    /// \details These events should not be produced manually by user.
    Container::HashSet<StandardLayout::Mapping> automaticEventTypes;

    std::array<EventUsageMap, static_cast<std::size_t> (PipelineType::COUNT)> eventProduction;

    std::array<EventUsageMap, static_cast<std::size_t> (PipelineType::COUNT)> eventConsumption;
};

/// \brief Helper class for task executor creation, that takes care of memory usage logging.
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
TaskConstructor &TaskConstructor::SetExecutor (Args... _args) noexcept
{
    static_assert (std::is_base_of_v<TaskExecutorBase<Executor>, Executor>);
    auto placeholder = heap.GetAllocationGroup ().PlaceOnTop ();
    Handling::Handle<Executor> handle {new (heap.Acquire (sizeof (Executor), alignof (Executor)))
                                           Executor {*this, std::forward<Args> (_args)...}};

    return SetExecutor (
        [handle] ()
        {
            handle->Execute ();
        });
}
} // namespace Emergence::Celerity
