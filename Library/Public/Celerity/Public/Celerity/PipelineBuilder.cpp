#include <Celerity/PipelineBuilder.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity
{
static Memory::UniqueString GetEventCleanerName (const StandardLayout::Mapping &_eventType) noexcept
{
    return Memory::UniqueString {EMERGENCE_BUILD_STRING (_eventType.GetName (), "Cleaner")};
}

class EventCleaner final : public TaskExecutorBase<EventCleaner>
{
public:
    EventCleaner (TaskConstructor &_constructor, const StandardLayout::Mapping &_eventType) noexcept;

    void Execute ();

private:
    ModifySequenceQuery modifyEvents;
};

EventCleaner::EventCleaner (TaskConstructor &_constructor, const StandardLayout::Mapping &_eventType) noexcept
    : modifyEvents (_constructor.ModifySequence (_eventType))
{
}

void EventCleaner::Execute ()
{
    auto cursor = modifyEvents.Execute ();
    while (*cursor)
    {
        ~cursor;
    }
}

TaskConstructor::TaskConstructor (TaskConstructor &&_other) noexcept
    : parent (_other.parent),
      task (std::move (_other.task)),
      heap (_other.heap.GetAllocationGroup ())
{
    _other.parent = nullptr;
}

TaskConstructor::~TaskConstructor () noexcept
{
    if (parent)
    {
        parent->FinishTaskRegistration (std::move (task));
    }
}

TaskConstructor &TaskConstructor::DependOn (Memory::UniqueString _taskOrCheckpoint) noexcept
{
    task.dependsOn.emplace (_taskOrCheckpoint);
    return *this;
}

TaskConstructor &TaskConstructor::MakeDependencyOf (Memory::UniqueString _taskOrCheckpoint) noexcept
{
    task.dependencyOf.emplace (_taskOrCheckpoint);
    return *this;
}

FetchSingletonQuery TaskConstructor::FetchSingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping).localRegistry.FetchSingleton (_typeMapping);
}

ModifySingletonQuery TaskConstructor::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    return ModifySingletonPartial (_typeMapping, {});
}

ModifySingletonQuery TaskConstructor::ModifySingletonPartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);
    ChangeTracker *changeTracker = BindChangeTracker (_typeMapping, _editedFields);

    return ModifySingletonQuery {
        view.localRegistry.ModifySingleton (_typeMapping),
        changeTracker ? view.RequestOnChangeEventInstances (parent->currentPipelineType, changeTracker) : nullptr,
        changeTracker};
}

InsertShortTermQuery TaskConstructor::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    if (parent->eventTypes.contains (_typeMapping))
    {
        if (parent->automaticEventTypes.contains (_typeMapping))
        {
            parent->anyErrorsDetected = true;
            EMERGENCE_LOG (ERROR, "Found insertion of automatic events with type \"", _typeMapping.GetName (),
                           "\". Automatic (on add, on remove and on change) events must not be inserted manually.");
        }
        else
        {
            RegisterEventProduction (_typeMapping);
        }
    }

    return parent->worldView->FindViewForType (_typeMapping).localRegistry.InsertShortTerm (_typeMapping);
}

FetchSequenceQuery TaskConstructor::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    WorldView &targetView = parent->worldView->FindViewForType (_typeMapping);

    if (parent->eventTypes.contains (_typeMapping))
    {
        if (&targetView != parent->worldView)
        {
            parent->anyErrorsDetected = true;
            EMERGENCE_LOG (ERROR, "Found fetch of events with type \"", _typeMapping.GetName (),
                           "\" from world view \"", parent->worldView->GetName (),
                           "\", but this event type is only allowed to be fetched in world view \"",
                           targetView.GetName (), "\".");
        }

        RegisterEventConsumption (_typeMapping);
    }

    return targetView.localRegistry.FetchSequence (_typeMapping);
}

ModifySequenceQuery TaskConstructor::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    if (parent->eventTypes.contains (_typeMapping) && !parent->postProcessingEvents)
    {
        parent->anyErrorsDetected = true;
        EMERGENCE_LOG (ERROR, "Found modification of events with type \"", _typeMapping.GetName (),
                       "\". Event modification is not allowed.");
    }

    return parent->worldView->FindViewForType (_typeMapping).localRegistry.ModifySequence (_typeMapping);
}

InsertLongTermQuery TaskConstructor::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);
    TrivialEventTriggerRow *eventsOnAdd = BindEventsOnAdd (_typeMapping);

    return InsertLongTermQuery {
        view.localRegistry.InsertLongTerm (_typeMapping),
        eventsOnAdd ? view.RequestOnAddEventInstances (parent->currentPipelineType, eventsOnAdd) : nullptr};
}

FetchValueQuery TaskConstructor::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                             const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping).localRegistry.FetchValue (_typeMapping, _keyFields);
}

ModifyValueQuery TaskConstructor::ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                               const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    return ModifyValuePartial (_typeMapping, _keyFields, {});
}

ModifyValueQuery TaskConstructor::ModifyValuePartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<StandardLayout::FieldId> &_keyFields,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifyValueQuery> (view.localRegistry.ModifyValue (_typeMapping, _keyFields), view,
                                                   _typeMapping, _editedFields);
}

EditValueQuery TaskConstructor::EditValue (const StandardLayout::Mapping &_typeMapping,
                                           const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    return EditValuePartial (_typeMapping, _keyFields, {});
}

EditValueQuery TaskConstructor::EditValuePartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<StandardLayout::FieldId> &_keyFields,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditValueQuery> (view.localRegistry.ModifyValue (_typeMapping, _keyFields), view,
                                               _typeMapping, _editedFields);
}

RemoveValueQuery TaskConstructor::RemoveValue (const StandardLayout::Mapping &_typeMapping,
                                               const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveValueQuery> (view.localRegistry.ModifyValue (_typeMapping, _keyFields), view,
                                                   _typeMapping);
}

FetchAscendingRangeQuery TaskConstructor::FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                               StandardLayout::FieldId _keyField) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping)
        .localRegistry.FetchAscendingRange (_typeMapping, _keyField);
}

ModifyAscendingRangeQuery TaskConstructor::ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    return ModifyAscendingRangePartial (_typeMapping, _keyField, {});
}

ModifyAscendingRangeQuery TaskConstructor::ModifyAscendingRangePartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifyAscendingRangeQuery> (
        view.localRegistry.ModifyAscendingRange (_typeMapping, _keyField), view, _typeMapping, _editedFields);
}

EditAscendingRangeQuery TaskConstructor::EditAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                             StandardLayout::FieldId _keyField) noexcept
{
    return EditAscendingRangePartial (_typeMapping, _keyField, {});
}

EditAscendingRangeQuery TaskConstructor::EditAscendingRangePartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditAscendingRangeQuery> (
        view.localRegistry.ModifyAscendingRange (_typeMapping, _keyField), view, _typeMapping, _editedFields);
}

RemoveAscendingRangeQuery TaskConstructor::RemoveAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveAscendingRangeQuery> (
        view.localRegistry.ModifyAscendingRange (_typeMapping, _keyField), view, _typeMapping);
}

FetchDescendingRangeQuery TaskConstructor::FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping)
        .localRegistry.FetchDescendingRange (_typeMapping, _keyField);
}

ModifyDescendingRangeQuery TaskConstructor::ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                   StandardLayout::FieldId _keyField) noexcept
{
    return ModifyDescendingRangePartial (_typeMapping, _keyField, {});
}

ModifyDescendingRangeQuery TaskConstructor::ModifyDescendingRangePartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifyDescendingRangeQuery> (
        view.localRegistry.ModifyDescendingRange (_typeMapping, _keyField), view, _typeMapping, _editedFields);
}

EditDescendingRangeQuery TaskConstructor::EditDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                               StandardLayout::FieldId _keyField) noexcept
{
    return EditDescendingRangePartial (_typeMapping, _keyField, {});
}

EditDescendingRangeQuery TaskConstructor::EditDescendingRangePartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditDescendingRangeQuery> (
        view.localRegistry.ModifyDescendingRange (_typeMapping, _keyField), view, _typeMapping, _editedFields);
}

RemoveDescendingRangeQuery TaskConstructor::RemoveDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                   StandardLayout::FieldId _keyField) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveDescendingRangeQuery> (
        view.localRegistry.ModifyDescendingRange (_typeMapping, _keyField), view, _typeMapping);
}

FetchSignalQuery TaskConstructor::FetchSignal (const StandardLayout::Mapping &_typeMapping,
                                               StandardLayout::FieldId _keyField,
                                               const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping)
        .localRegistry.FetchSignal (_typeMapping, _keyField, _signaledValue);
}

ModifySignalQuery TaskConstructor::ModifySignal (const StandardLayout::Mapping &_typeMapping,
                                                 StandardLayout::FieldId _keyField,
                                                 const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    return ModifySignalPartial (_typeMapping, _keyField, _signaledValue, {});
}

ModifySignalQuery TaskConstructor::ModifySignalPartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifySignalQuery> (
        view.localRegistry.ModifySignal (_typeMapping, _keyField, _signaledValue), view, _typeMapping, _editedFields);
}

EditSignalQuery TaskConstructor::EditSignal (const StandardLayout::Mapping &_typeMapping,
                                             StandardLayout::FieldId _keyField,
                                             const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    return EditSignalPartial (_typeMapping, _keyField, _signaledValue, {});
}

EditSignalQuery TaskConstructor::EditSignalPartial (
    const StandardLayout::Mapping &_typeMapping,
    StandardLayout::FieldId _keyField,
    const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditSignalQuery> (
        view.localRegistry.ModifySignal (_typeMapping, _keyField, _signaledValue), view, _typeMapping, _editedFields);
}

RemoveSignalQuery TaskConstructor::RemoveSignal (const StandardLayout::Mapping &_typeMapping,
                                                 StandardLayout::FieldId _keyField,
                                                 const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveSignalQuery> (
        view.localRegistry.ModifySignal (_typeMapping, _keyField, _signaledValue), view, _typeMapping);
}

FetchShapeIntersectionQuery TaskConstructor::FetchShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping)
        .localRegistry.FetchShapeIntersection (_typeMapping, _dimensions);
}

ModifyShapeIntersectionQuery TaskConstructor::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    return ModifyShapeIntersectionPartial (_typeMapping, _dimensions, {});
}

ModifyShapeIntersectionQuery TaskConstructor::ModifyShapeIntersectionPartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<Warehouse::Dimension> &_dimensions,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifyShapeIntersectionQuery> (
        view.localRegistry.ModifyShapeIntersection (_typeMapping, _dimensions), view, _typeMapping, _editedFields);
}

EditShapeIntersectionQuery TaskConstructor::EditShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    return EditShapeIntersectionPartial (_typeMapping, _dimensions, {});
}

EditShapeIntersectionQuery TaskConstructor::EditShapeIntersectionPartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<Warehouse::Dimension> &_dimensions,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditShapeIntersectionQuery> (
        view.localRegistry.ModifyShapeIntersection (_typeMapping, _dimensions), view, _typeMapping, _editedFields);
}

RemoveShapeIntersectionQuery TaskConstructor::RemoveShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveShapeIntersectionQuery> (
        view.localRegistry.ModifyShapeIntersection (_typeMapping, _dimensions), view, _typeMapping);
}

FetchRayIntersectionQuery TaskConstructor::FetchRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    RegisterReadAccess (_typeMapping.GetName ());
    return parent->worldView->FindViewForType (_typeMapping)
        .localRegistry.FetchRayIntersection (_typeMapping, _dimensions);
}

ModifyRayIntersectionQuery TaskConstructor::ModifyRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    return ModifyRayIntersectionPartial (_typeMapping, _dimensions, {});
}

ModifyRayIntersectionQuery TaskConstructor::ModifyRayIntersectionPartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<Warehouse::Dimension> &_dimensions,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructModifyQuery<ModifyRayIntersectionQuery> (
        view.localRegistry.ModifyRayIntersection (_typeMapping, _dimensions), view, _typeMapping, _editedFields);
}

EditRayIntersectionQuery TaskConstructor::EditRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    return EditRayIntersectionPartial (_typeMapping, _dimensions, {});
}

EditRayIntersectionQuery TaskConstructor::EditRayIntersectionPartial (
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<Warehouse::Dimension> &_dimensions,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructEditQuery<EditRayIntersectionQuery> (
        view.localRegistry.ModifyRayIntersection (_typeMapping, _dimensions), view, _typeMapping, _editedFields);
}

RemoveRayIntersectionQuery TaskConstructor::RemoveRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    RegisterWriteAccess (_typeMapping.GetName ());
    WorldView &view = parent->worldView->FindViewForType (_typeMapping);

    return ConstructRemoveQuery<RemoveRayIntersectionQuery> (
        view.localRegistry.ModifyRayIntersection (_typeMapping, _dimensions), view, _typeMapping);
}

TaskConstructor &TaskConstructor::SetExecutor (std::function<void ()> _executor) noexcept
{
    task.executor = std::move (_executor);
    return *this;
}

TaskConstructor::TaskConstructor (PipelineBuilder *_parent, Memory::UniqueString _name) noexcept
    : parent (_parent),
      heap {Memory::Profiler::AllocationGroup {_name}}
{
    EMERGENCE_ASSERT (parent);
    task.name = _name;
}

void TaskConstructor::RegisterReadAccess (Memory::UniqueString _resourceName) noexcept
{
    if (!task.writeAccess.contains (_resourceName))
    {
        task.readAccess.emplace (_resourceName);
    }
}

void TaskConstructor::RegisterWriteAccess (Memory::UniqueString _resourceName) noexcept
{
    task.writeAccess.emplace (_resourceName);
    task.readAccess.erase (_resourceName);
}

template <typename WrappedQuery, typename SourceQuery>
WrappedQuery TaskConstructor::ConstructModifyQuery (
    SourceQuery _source,
    WorldView &_view,
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    TrivialEventTriggerRow *eventsOnRemove = BindEventsOnRemove (_typeMapping);
    ChangeTracker *changeTracker = BindChangeTracker (_typeMapping, _editedFields);
    return WrappedQuery {
        _source,
        eventsOnRemove ? _view.RequestOnRemoveEventInstances (parent->currentPipelineType, eventsOnRemove) : nullptr,
        changeTracker ? _view.RequestOnChangeEventInstances (parent->currentPipelineType, changeTracker) : nullptr,
        changeTracker};
}

template <typename WrappedQuery, typename SourceQuery>
WrappedQuery TaskConstructor::ConstructEditQuery (
    SourceQuery _source,
    WorldView &_view,
    const StandardLayout::Mapping &_typeMapping,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    ChangeTracker *changeTracker = BindChangeTracker (_typeMapping, _editedFields);
    return WrappedQuery {
        _source, nullptr,
        changeTracker ? _view.RequestOnChangeEventInstances (parent->currentPipelineType, changeTracker) : nullptr,
        changeTracker};
}

template <typename WrappedQuery, typename SourceQuery>
WrappedQuery TaskConstructor::ConstructRemoveQuery (SourceQuery _source,
                                                    WorldView &_view,
                                                    const StandardLayout::Mapping &_typeMapping) noexcept
{
    TrivialEventTriggerRow *eventsOnRemove = BindEventsOnRemove (_typeMapping);
    return WrappedQuery {
        _source,
        eventsOnRemove ? _view.RequestOnRemoveEventInstances (parent->currentPipelineType, eventsOnRemove) : nullptr,
        nullptr, nullptr};
}

TrivialEventTriggerRow *TaskConstructor::BindTrivialEvents (Container::TypedOrderedPool<TrivialEventTriggerRow> &_rows,
                                                            const StandardLayout::Mapping &_trackedType) noexcept
{
    for (TrivialEventTriggerRow &row : _rows)
    {
        if (!row.Empty () && row.Front ().GetTrackedType () == _trackedType)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                RegisterWriteAccess (trigger.GetEventType ().GetName ());
                RegisterEventProduction (trigger.GetEventType ());
            }

            return &row;
        }
    }

    return nullptr;
}

TrivialEventTriggerRow *TaskConstructor::BindEventsOnAdd (const StandardLayout::Mapping &_trackedType) noexcept
{
    World::EventScheme &eventScheme =
        parent->worldView->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];
    return BindTrivialEvents (eventScheme.onAdd, _trackedType);
}

TrivialEventTriggerRow *TaskConstructor::BindEventsOnRemove (const StandardLayout::Mapping &_trackedType) noexcept
{
    World::EventScheme &eventScheme =
        parent->worldView->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];
    return BindTrivialEvents (eventScheme.onRemove, _trackedType);
}

ChangeTracker *TaskConstructor::BindChangeTracker (
    const StandardLayout::Mapping &_trackedType,
    const Container::Vector<StandardLayout::FieldId> &_editedFields) noexcept
{
    World::EventScheme &eventScheme =
        parent->worldView->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];

    for (ChangeTracker &tracker : eventScheme.changeTrackers)
    {
        if (tracker.GetTrackedType () == _trackedType)
        {
            for (OnChangeEventTrigger *trigger : tracker.GetEventTriggers ())
            {
                // We treat empty field edition array as full object edition.
                bool editingTrackedFields = _editedFields.empty ();

                if (!_editedFields.empty ())
                {
                    for (StandardLayout::FieldId field : _editedFields)
                    {
                        if (trigger->IsFieldTracked (field))
                        {
                            editingTrackedFields = true;
                            break;
                        }
                    }
                }

                if (editingTrackedFields)
                {
                    RegisterWriteAccess (trigger->GetEventType ().GetName ());
                    RegisterEventProduction (trigger->GetEventType ());
                }
            }

            return &tracker;
        }
    }

    return nullptr;
}

void TaskConstructor::RegisterEventProduction (const StandardLayout::Mapping &_eventType) noexcept
{
    // Check that message production is not forbidden by parent views.
    WorldView *viewToCheck = parent->worldView->parent;

    while (viewToCheck)
    {
        if (viewToCheck->eventProductionForbiddenInChildren.contains (_eventType))
        {
            parent->anyErrorsDetected = true;
            EMERGENCE_LOG (ERROR, "Events of type \"", _eventType.GetName (), "\" can not be produced in world view \"",
                           parent->worldView->GetName (), "\" as its parent view \"", viewToCheck->GetName (),
                           "\" forbids production in children views. It usually happens when production would result "
                           "in partial consumption of events.");
            break;
        }

        viewToCheck = viewToCheck->parent;
    }

    PipelineBuilder::EventUsageMap &productionMap =
        parent->eventProduction[static_cast<std::size_t> (parent->currentPipelineType)];

    auto iterator = productionMap.find (_eventType);
    if (iterator != productionMap.end ())
    {
        iterator->second.emplace (task.name);
    }
    else
    {
        parent->anyErrorsDetected = true;
        EMERGENCE_LOG (ERROR, "Events of type \"", _eventType.GetName (), "\" can not be produced in pipeline with \"",
                       GetPipelineTypeName (parent->currentPipelineType), "\" type.");
    }
}

void TaskConstructor::RegisterEventConsumption (const StandardLayout::Mapping &_eventType) noexcept
{
    PipelineBuilder::EventUsageMap &consumptionMap =
        parent->eventConsumption[static_cast<std::size_t> (parent->currentPipelineType)];

    auto iterator = consumptionMap.find (_eventType);
    if (iterator != consumptionMap.end ())
    {
        iterator->second.emplace (task.name);
    }
    else
    {
        parent->anyErrorsDetected = true;
        EMERGENCE_LOG (ERROR, "Events of type \"", _eventType.GetName (), "\" can not be consumed in pipeline with \"",
                       GetPipelineTypeName (parent->currentPipelineType), "\" type.");
    }
}

PipelineBuilder::PipelineBuilder (WorldView *_targetWorldView) noexcept
    : worldView (_targetWorldView),
      registeredResources (GetBuildTimeAllocationGroup ()),
      eventTypes (GetBuildTimeAllocationGroup ()),
      sharedEventTypes (GetBuildTimeAllocationGroup ()),
      automaticEventTypes (GetBuildTimeAllocationGroup ()),
      eventProduction ({
          EventUsageMap {GetBuildTimeAllocationGroup ()},
          EventUsageMap {GetBuildTimeAllocationGroup ()},
          EventUsageMap {GetBuildTimeAllocationGroup ()},
      }),
      eventConsumption ({
          EventUsageMap {GetBuildTimeAllocationGroup ()},
          EventUsageMap {GetBuildTimeAllocationGroup ()},
          EventUsageMap {GetBuildTimeAllocationGroup ()},
      })
{
    EMERGENCE_ASSERT (worldView);
    for (const World::EventScheme &scheme : worldView->world->eventSchemes)
    {
        ImportEventScheme (scheme);
    }
}

bool PipelineBuilder::Begin (Memory::UniqueString _id, PipelineType _type) noexcept
{
    switch (_type)
    {
    case PipelineType::NORMAL:
        if (worldView->normalPipeline)
        {
            // Normal pipeline for this world is already built.
            return false;
        }

        break;
    case PipelineType::FIXED:
        if (worldView->fixedPipeline)
        {
            // Fixed pipeline for this world is already built.
            return false;
        }

        break;
    case PipelineType::CUSTOM:
        break;

    case PipelineType::COUNT:
        return false;
    }

    currentPipelineId = _id;
    currentPipelineType = _type;

    currentPipelineAllocationGroup =
        Memory::Profiler::AllocationGroup {worldView->pipelinePool.GetAllocationGroup (), currentPipelineId};

    // Everything else should be cleared in ::End.
    return true;
}

TaskConstructor PipelineBuilder::AddTask (Memory::UniqueString _name) noexcept
{
    auto placeholder = currentPipelineAllocationGroup.PlaceOnTop ();
    return {this, _name};
}

void PipelineBuilder::AddCheckpoint (Memory::UniqueString _name) noexcept
{
    taskRegister.RegisterCheckpoint (_name);
}

void PipelineBuilder::AddCheckpointDependency (Memory::UniqueString _from, Memory::UniqueString _to) noexcept
{
    taskRegister.RegisterCheckpointDependency (_from, _to);
}

Flow::TaskRegister::VisualGroupNodePlaced PipelineBuilder::OpenVisualGroup (Container::String _name) noexcept
{
    return taskRegister.OpenVisualGroup (std::move (_name));
}

Pipeline *PipelineBuilder::End (VisualGraph::Graph *_visualGraphOutput, bool _exportResourcesToGraph) noexcept
{
    EventUsageMap &productionMap = eventProduction[static_cast<std::size_t> (currentPipelineType)];
    EventUsageMap &consumptionMap = eventConsumption[static_cast<std::size_t> (currentPipelineType)];

    switch (currentPipelineType)
    {
    case PipelineType::NORMAL:
    case PipelineType::FIXED:
        PostProcessContinuousEventRoutine (productionMap, consumptionMap);
        break;

    case PipelineType::CUSTOM:
        PostProcessLocalEventRoutine (productionMap, consumptionMap);
        break;

    case PipelineType::COUNT:
        EMERGENCE_ASSERT (false);
        break;
    }

    PostProcessSharedEventRoutine (consumptionMap);
    Pipeline *newPipeline = nullptr;

    if (!anyErrorsDetected)
    {
        if (_visualGraphOutput)
        {
            *_visualGraphOutput = taskRegister.ExportVisual (_exportResourcesToGraph);
        }

        newPipeline = worldView->AddPipeline (currentPipelineId, currentPipelineType, taskRegister.ExportCollection ());
    }

    taskRegister.Clear ();
    registeredResources.clear ();
    anyErrorsDetected = false;

    for (auto &[eventType, producers] : productionMap)
    {
        producers.clear ();
    }

    for (auto &[eventType, consumers] : consumptionMap)
    {
        consumers.clear ();
    }

    return newPipeline;
}

Memory::Profiler::AllocationGroup PipelineBuilder::GetBuildTimeAllocationGroup () noexcept
{
    return {worldView->pipelinePool.GetAllocationGroup (), Memory::UniqueString {"PipelineBuilder"}};
}

void PipelineBuilder::ImportEventScheme (const World::EventScheme &_scheme) noexcept
{
    auto registerEvent = [this] (const StandardLayout::Mapping &_type, EventRoute _route)
    {
        eventTypes.emplace (_type);
        const PipelineType producingPipeline = GetEventProducingPipeline (_route);
        const PipelineType consumingPipeline = GetEventConsumingPipeline (_route);

        eventProduction[static_cast<std::size_t> (producingPipeline)].emplace (
            _type, Container::HashSet<Memory::UniqueString> {GetBuildTimeAllocationGroup ()});

        eventConsumption[static_cast<std::size_t> (consumingPipeline)].emplace (
            _type, Container::HashSet<Memory::UniqueString> {GetBuildTimeAllocationGroup ()});

        if (producingPipeline != consumingPipeline)
        {
            sharedEventTypes.emplace (_type);
        }
    };

    for (const World::CustomEventInfo &info : _scheme.custom)
    {
        registerEvent (info.type, info.route);
    }

    for (const TrivialEventTriggerRow &row : _scheme.onAdd)
    {
        for (const TrivialEventTrigger &trigger : row)
        {
            registerEvent (trigger.GetEventType (), trigger.GetRoute ());
            automaticEventTypes.emplace (trigger.GetEventType ());
        }
    }

    for (const TrivialEventTriggerRow &row : _scheme.onRemove)
    {
        for (const TrivialEventTrigger &trigger : row)
        {
            registerEvent (trigger.GetEventType (), trigger.GetRoute ());
            automaticEventTypes.emplace (trigger.GetEventType ());
        }
    }

    for (const OnChangeEventTrigger &trigger : _scheme.onChange)
    {
        registerEvent (trigger.GetEventType (), trigger.GetRoute ());
        automaticEventTypes.emplace (trigger.GetEventType ());
    }
}

void PipelineBuilder::FinishTaskRegistration (Flow::Task _task) noexcept
{
    for (Memory::UniqueString resource : _task.readAccess)
    {
        if (registeredResources.emplace (resource).second)
        {
            taskRegister.RegisterResource (resource);
        }
    }

    for (Memory::UniqueString resource : _task.writeAccess)
    {
        if (registeredResources.emplace (resource).second)
        {
            taskRegister.RegisterResource (resource);
        }
    }

    taskRegister.RegisterTask (std::move (_task));
}

void PipelineBuilder::PostProcessContinuousEventRoutine (const PipelineBuilder::EventUsageMap &_production,
                                                         const PipelineBuilder::EventUsageMap &_consumption)
{
    postProcessingEvents = true;
    const Flow::TaskRegister::UnwrappedDependencyMap dependencyMap = taskRegister.ExportUnwrappedDependencyMap ();

    for (const auto &[eventType, producers] : _production)
    {
        if (sharedEventTypes.contains (eventType))
        {
            continue;
        }

        // Skip events that are not local for this view: they can only be produced in this pipeline.
        if (!worldView->localRegistry.IsTypeUsed (eventType))
        {
            continue;
        }

        auto consumptionIterator = _consumption.find (eventType);
        if (consumptionIterator == _consumption.end () || consumptionIterator->second.empty ())
        {
            continue;
        }

        TaskConstructor constructor = AddTask (GetEventCleanerName (eventType));
        constructor.SetExecutor<EventCleaner> (eventType);

        for (const Memory::UniqueString &producerTask : producers)
        {
            constructor.MakeDependencyOf (producerTask);
        }

        for (const Memory::UniqueString &consumerTask : consumptionIterator->second)
        {
            // Check whether this consumer belongs to "current execution consumers" category.

            // If there is no producers, then events are being sent from other views.
            // In this case, we treat all consumers as "previous execution consumers" category.
            if (!producers.empty ())
            {
                auto consumerDependencyIterator = dependencyMap.find (consumerTask);
                if (consumerDependencyIterator != dependencyMap.end ())
                {
                    bool dependsOnAllProducers = true;
                    bool dependsOnAnyProducer = false;

                    for (const Memory::UniqueString &producerTask : producers)
                    {
                        const bool dependsOnProducer = consumerDependencyIterator->second.contains (producerTask);
                        dependsOnAllProducers &= dependsOnProducer;
                        dependsOnAnyProducer |= dependsOnProducer;

                        if (!dependsOnAllProducers && dependsOnAnyProducer)
                        {
                            // Error found, stop processing.
                            break;
                        }
                    }

                    if (dependsOnAllProducers)
                    {
                        // Belongs to "current execution consumers" category.
                        worldView->eventProductionForbiddenInChildren.emplace (eventType);
                        continue;
                    }

                    if (dependsOnAnyProducer)
                    {
                        Container::StringBuilder builder = EMERGENCE_BEGIN_BUILDING_STRING (
                            "Task \"", consumerTask, "\" consumes events of type \"", eventType.GetName (),
                            "\", but graph ensures that task is executed only after some of the producers, instead of "
                            "all. List of dependency producers: ");

                        for (const Memory::UniqueString &producerTask : producers)
                        {
                            if (consumerDependencyIterator->second.contains (producerTask))
                            {
                                builder.Append ("\"", producerTask, "\" ");
                            }
                        }

                        builder.Append (". List of dependant or unspecified producers: ");
                        for (const Memory::UniqueString &producerTask : producers)
                        {
                            if (!consumerDependencyIterator->second.contains (producerTask))
                            {
                                builder.Append ("\"", producerTask, "\" ");
                            }
                        }

                        EMERGENCE_LOG (ERROR, builder.Get ());
                        anyErrorsDetected = true;
                        continue;
                    }
                }
            }

            // Check whether this consumer belongs to "previous execution consumers" category.

            bool dependencyOfAllProducers = true;
            for (const Memory::UniqueString &producerTask : producers)
            {
                bool dependency = false;
                auto producerDependencyIterator = dependencyMap.find (producerTask);

                if (producerDependencyIterator != dependencyMap.end ())
                {
                    dependency = producerDependencyIterator->second.contains (consumerTask);
                }

                if (!dependency)
                {
                    dependencyOfAllProducers = false;
                    break;
                }
            }

            if (dependencyOfAllProducers)
            {
                // Belongs to "previous execution consumers" category.
                constructor.DependOn (consumerTask);
                continue;
            }

            Container::StringBuilder builder = EMERGENCE_BEGIN_BUILDING_STRING (
                "Task \"", consumerTask, "\" consumes events of type \"", eventType.GetName (),
                "\", but it is not correctly positioned in graph. Graph must ensure that this task is always executed "
                "before all producers or after all producers: ");

            for (const Memory::UniqueString &producerTask : producers)
            {
                builder.Append ("\"", producerTask, "\" ");
            }

            EMERGENCE_LOG (ERROR, builder.Get ());
            anyErrorsDetected = true;
        }
    }

    postProcessingEvents = false;
}

void PipelineBuilder::PostProcessLocalEventRoutine (const PipelineBuilder::EventUsageMap &_production,
                                                    const PipelineBuilder::EventUsageMap &_consumption)
{
    postProcessingEvents = true;
    const Flow::TaskRegister::UnwrappedDependencyMap dependencyMap = taskRegister.ExportUnwrappedDependencyMap ();

    for (const auto &[eventType, producers] : _production)
    {
        if (sharedEventTypes.contains (eventType))
        {
            continue;
        }

        // Skip events that are not local for this view: they can only be produced in this pipeline.
        if (!worldView->localRegistry.IsTypeUsed (eventType))
        {
            continue;
        }

        auto consumptionIterator = _consumption.find (eventType);
        if (consumptionIterator == _consumption.end () || consumptionIterator->second.empty ())
        {
            EMERGENCE_LOG (ERROR, "Events of type \"", eventType.GetName (), "\" are produced, but never consumed.");
            anyErrorsDetected = true;
            continue;
        }

        TaskConstructor constructor = AddTask (GetEventCleanerName (eventType));
        constructor.SetExecutor<EventCleaner> (eventType);

        for (const Memory::UniqueString &consumerTask : consumptionIterator->second)
        {
            constructor.DependOn (consumerTask);

            // Verify that consumer task is always executed after all producers.

            auto dependencyIterator = dependencyMap.find (consumerTask);
            if (dependencyIterator != dependencyMap.end ())
            {
                for (const Memory::UniqueString &producerTask : producers)
                {
                    if (!dependencyIterator->second.contains (producerTask))
                    {
                        EMERGENCE_LOG (
                            ERROR, "Task \"", consumerTask, "\" consumes event \"", eventType.GetName (),
                            "\", but graph does not guarantees that this task is executed after producer task \"",
                            producerTask, "\" and therefore is not guaranteed to consume all events.");

                        anyErrorsDetected = true;
                    }
                }
            }
            else
            {
                EMERGENCE_LOG (ERROR, "Task \"", consumerTask, "\" consumes event \"", eventType.GetName (),
                               "\", but has no dependencies and therefore is not guaranteed to consume all events.");
                anyErrorsDetected = true;
            }
        }
    }

    postProcessingEvents = false;
}

void PipelineBuilder::PostProcessSharedEventRoutine (const PipelineBuilder::EventUsageMap &_consumption)
{
    postProcessingEvents = true;
    for (const StandardLayout::Mapping &eventType : sharedEventTypes)
    {
        // Skip events that are not local for this view: they can only be produced in this pipeline.
        if (!worldView->localRegistry.IsTypeUsed (eventType))
        {
            continue;
        }

        auto consumptionIterator = _consumption.find (eventType);
        if (consumptionIterator != _consumption.end ())
        {
            TaskConstructor constructor = AddTask (GetEventCleanerName (eventType));
            constructor.SetExecutor<EventCleaner> (eventType);

            for (const Memory::UniqueString &consumerTask : consumptionIterator->second)
            {
                constructor.DependOn (consumerTask);
            }
        }
    }

    postProcessingEvents = false;
}
} // namespace Emergence::Celerity
