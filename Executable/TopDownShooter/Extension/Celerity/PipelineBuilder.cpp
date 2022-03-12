#include <cassert>

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

void TaskConstructor::DependOn (Memory::UniqueString _taskOrCheckpoint) noexcept
{
    task.dependsOn.emplace (_taskOrCheckpoint);
}

void TaskConstructor::MakeDependencyOf (Memory::UniqueString _taskOrCheckpoint) noexcept
{
    task.dependencyOf.emplace (_taskOrCheckpoint);
}

FetchSingletonQuery TaskConstructor::FetchSingleton (const StandardLayout::Mapping &_typeMapping)
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchSingleton (_typeMapping);
}

ModifySingletonQuery TaskConstructor::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifySingletonQuery {parent->world->registry.ModifySingleton (_typeMapping),
                                 BindChangeTracker (_typeMapping)};
}

InsertShortTermQuery TaskConstructor::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
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

    return parent->world->registry.InsertShortTerm (_typeMapping);
}

FetchSequenceQuery TaskConstructor::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    if (parent->eventTypes.contains (_typeMapping))
    {
        RegisterEventConsumption (_typeMapping);
    }

    return parent->world->registry.FetchSequence (_typeMapping);
}

ModifySequenceQuery TaskConstructor::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    if (parent->eventTypes.contains (_typeMapping) && !parent->postProcessingEvents)
    {
        parent->anyErrorsDetected = true;
        EMERGENCE_LOG (ERROR, "Found modification of events with type \"", _typeMapping.GetName (),
                       "\". Event modification is not allowed.");
    }

    return parent->world->registry.ModifySequence (_typeMapping);
}

InsertLongTermQuery TaskConstructor::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return InsertLongTermQuery {parent->world->registry.InsertLongTerm (_typeMapping), BindEventsOnAdd (_typeMapping)};
}

FetchValueQuery TaskConstructor::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                             const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchValue (_typeMapping, _keyFields);
}

ModifyValueQuery TaskConstructor::ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                               const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifyValueQuery {parent->world->registry.ModifyValue (_typeMapping, _keyFields),
                             BindEventsOnRemove (_typeMapping), BindChangeTracker (_typeMapping)};
}

FetchAscendingRangeQuery TaskConstructor::FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                               StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchAscendingRange (_typeMapping, _keyField);
}

ModifyAscendingRangeQuery TaskConstructor::ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifyAscendingRangeQuery {parent->world->registry.ModifyAscendingRange (_typeMapping, _keyField),
                                      BindEventsOnRemove (_typeMapping), BindChangeTracker (_typeMapping)};
}

FetchDescendingRangeQuery TaskConstructor::FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchDescendingRange (_typeMapping, _keyField);
}

ModifyDescendingRangeQuery TaskConstructor::ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                   StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifyDescendingRangeQuery {parent->world->registry.ModifyDescendingRange (_typeMapping, _keyField),
                                       BindEventsOnRemove (_typeMapping), BindChangeTracker (_typeMapping)};
}

FetchShapeIntersectionQuery TaskConstructor::FetchShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchShapeIntersection (_typeMapping, _dimensions);
}

ModifyShapeIntersectionQuery TaskConstructor::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifyShapeIntersectionQuery {parent->world->registry.ModifyShapeIntersection (_typeMapping, _dimensions),
                                         BindEventsOnRemove (_typeMapping), BindChangeTracker (_typeMapping)};
}

FetchRayIntersectionQuery TaskConstructor::FetchRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace (_typeMapping.GetName ());
    return parent->world->registry.FetchRayIntersection (_typeMapping, _dimensions);
}

ModifyRayIntersectionQuery TaskConstructor::ModifyRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace (_typeMapping.GetName ());
    return ModifyRayIntersectionQuery {parent->world->registry.ModifyRayIntersection (_typeMapping, _dimensions),
                                       BindEventsOnRemove (_typeMapping), BindChangeTracker (_typeMapping)};
}

void TaskConstructor::SetExecutor (std::function<void ()> _executor) noexcept
{
    task.executor = std::move (_executor);
}

World *TaskConstructor::GetWorld () const noexcept
{
    return parent->world;
}

TaskConstructor &TaskConstructor::operator= (TaskConstructor &&_other) noexcept
{
    if (this != &_other)
    {
        this->~TaskConstructor ();
        new (this) TaskConstructor (std::move (_other));
    }

    return *this;
}

TaskConstructor::TaskConstructor (PipelineBuilder *_parent, Memory::UniqueString _name) noexcept
    : parent (_parent),
      heap {Memory::Profiler::AllocationGroup {_name}}
{
    assert (parent);
    task.name = _name;
}

TrivialEventTriggerRow *TaskConstructor::BindTrivialEvents (Container::Vector<TrivialEventTriggerRow> &_rows,
                                                            const StandardLayout::Mapping &_trackedType) noexcept
{
    for (TrivialEventTriggerRow &row : _rows)
    {
        if (!row.Empty () && row.Front ().GetTrackedType () == _trackedType)
        {
            for (const TrivialEventTrigger &trigger : row)
            {
                task.writeAccess.emplace (trigger.GetEventType ().GetName ());
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
        parent->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];
    return BindTrivialEvents (eventScheme.onAdd, _trackedType);
}

TrivialEventTriggerRow *TaskConstructor::BindEventsOnRemove (const StandardLayout::Mapping &_trackedType) noexcept
{
    World::EventScheme &eventScheme =
        parent->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];
    return BindTrivialEvents (eventScheme.onRemove, _trackedType);
}

ChangeTracker *TaskConstructor::BindChangeTracker (const StandardLayout::Mapping &_trackedType) noexcept
{
    World::EventScheme &eventScheme =
        parent->world->eventSchemes[static_cast<std::size_t> (parent->currentPipelineType)];

    for (ChangeTracker &tracker : eventScheme.changeTrackers)
    {
        if (tracker.GetTrackedType () == _trackedType)
        {
            for (OnChangeEventTrigger *trigger : tracker.GetEventTriggers ())
            {
                task.writeAccess.emplace (trigger->GetEventType ().GetName ());
                RegisterEventProduction (trigger->GetEventType ());
            }

            return &tracker;
        }
    }

    return nullptr;
}

void TaskConstructor::RegisterEventProduction (const StandardLayout::Mapping &_eventType) noexcept
{
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

PipelineBuilder::PipelineBuilder (World *_targetWorld) noexcept
    : world (_targetWorld),
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
    assert (world);
    for (const World::EventScheme &scheme : world->eventSchemes)
    {
        ImportEventScheme (scheme);
    }
}

bool PipelineBuilder::Begin (Memory::UniqueString _id, PipelineType _type) noexcept
{
    switch (_type)
    {
    case PipelineType::NORMAL:
        if (world->normalPipeline)
        {
            // Normal pipeline for this world is already built.
            return false;
        }

        break;
    case PipelineType::FIXED:
        if (world->fixedPipeline)
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
        Memory::Profiler::AllocationGroup {world->pipelinePool.GetAllocationGroup (), currentPipelineId};

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

Pipeline *PipelineBuilder::End (std::size_t _maximumChildThreads) noexcept
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
        assert (false);
        break;
    }

    PostProcessSharedEventRoutine (consumptionMap);
    Pipeline *newPipeline = nullptr;

    if (!anyErrorsDetected)
    {
        newPipeline = world->AddPipeline (currentPipelineId, currentPipelineType, taskRegister.ExportCollection (),
                                          _maximumChildThreads);
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
    return {world->pipelinePool.GetAllocationGroup (), Memory::UniqueString {"PipelineBuilder"}};
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

        auto consumptionIterator = _consumption.find (eventType);
        if (consumptionIterator == _consumption.end () || consumptionIterator->second.empty ())
        {
            EMERGENCE_LOG (ERROR, "Events of type \"", eventType.GetName (), "\" are produced, but never consumed.");
            anyErrorsDetected = true;
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
                    continue;
                }

                if (dependsOnAnyProducer)
                {
                    Container::StringBuilder builder = EMERGENCE_BEGIN_BUILDING_STRING (
                        "Task \"", consumerTask, "\" consumes events of type \"", eventType.GetName (),
                        "\", but graph ensures that task is executed only after some of the producers, instead of "
                        "all: ");

                    for (const Memory::UniqueString &producerTask : producers)
                    {
                        builder.Append ("\"", producerTask, "\" ");
                    }

                    EMERGENCE_LOG (ERROR, builder.Get ());
                    anyErrorsDetected = true;
                    continue;
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
