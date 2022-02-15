#include <cassert>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
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
    task.dependsOn.emplace_back (_taskOrCheckpoint);
}

void TaskConstructor::MakeDependencyOf (Memory::UniqueString _taskOrCheckpoint) noexcept
{
    task.dependencyOf.emplace_back (_taskOrCheckpoint);
}

FetchSingletonQuery TaskConstructor::FetchSingleton (const StandardLayout::Mapping &_typeMapping)
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchSingleton (_typeMapping);
}

ModifySingletonQuery TaskConstructor::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifySingletonQuery {parent->world->registry.ModifySingleton (_typeMapping),
                                 FindChangeTracker (_typeMapping)};
}

InsertShortTermQuery TaskConstructor::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.InsertShortTerm (_typeMapping);
}

FetchSequenceQuery TaskConstructor::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchSequence (_typeMapping);
}

ModifySequenceQuery TaskConstructor::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifySequence (_typeMapping);
}

InsertLongTermQuery TaskConstructor::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    TrivialEventTriggerRow *eventsOnAdd = nullptr;

    for (TrivialEventTriggerRow &row : parent->world->eventOnAdd)
    {
        if (!row.Empty () && row.Front ().GetTrackedType () == _typeMapping)
        {
            eventsOnAdd = &row;
            break;
        }
    }

    return InsertLongTermQuery {parent->world->registry.InsertLongTerm (_typeMapping), eventsOnAdd};
}

FetchValueQuery TaskConstructor::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                             const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchValue (_typeMapping, _keyFields);
}

ModifyValueQuery TaskConstructor::ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                               const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifyValueQuery {parent->world->registry.ModifyValue (_typeMapping, _keyFields),
                             FindEventsOnRemove (_typeMapping), FindChangeTracker (_typeMapping)};
}

FetchAscendingRangeQuery TaskConstructor::FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                               StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchAscendingRange (_typeMapping, _keyField);
}

ModifyAscendingRangeQuery TaskConstructor::ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifyAscendingRangeQuery {parent->world->registry.ModifyAscendingRange (_typeMapping, _keyField),
                                      FindEventsOnRemove (_typeMapping), FindChangeTracker (_typeMapping)};
}

FetchDescendingRangeQuery TaskConstructor::FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                 StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchDescendingRange (_typeMapping, _keyField);
}

ModifyDescendingRangeQuery TaskConstructor::ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                   StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifyDescendingRangeQuery {parent->world->registry.ModifyDescendingRange (_typeMapping, _keyField),
                                       FindEventsOnRemove (_typeMapping), FindChangeTracker (_typeMapping)};
}

FetchShapeIntersectionQuery TaskConstructor::FetchShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchShapeIntersection (_typeMapping, _dimensions);
}

ModifyShapeIntersectionQuery TaskConstructor::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifyShapeIntersectionQuery {parent->world->registry.ModifyShapeIntersection (_typeMapping, _dimensions),
                                         FindEventsOnRemove (_typeMapping), FindChangeTracker (_typeMapping)};
}

FetchRayIntersectionQuery TaskConstructor::FetchRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchRayIntersection (_typeMapping, _dimensions);
}

ModifyRayIntersectionQuery TaskConstructor::ModifyRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return ModifyRayIntersectionQuery {parent->world->registry.ModifyRayIntersection (_typeMapping, _dimensions),
                                       FindEventsOnRemove (_typeMapping), FindChangeTracker (_typeMapping)};
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

TrivialEventTriggerRow *TaskConstructor::FindEventsOnRemove (const StandardLayout::Mapping &_trackedType) const noexcept
{
    for (TrivialEventTriggerRow &row : parent->world->eventOnRemove)
    {
        if (!row.Empty () && row.Front ().GetTrackedType () == _trackedType)
        {
            return &row;
        }
    }

    return nullptr;
}

ChangeTracker *TaskConstructor::FindChangeTracker (const StandardLayout::Mapping &_trackedType) const noexcept
{
    for (ChangeTracker &tracker : parent->world->changeTrackers)
    {
        if (tracker.GetTrackedType () == _trackedType)
        {
            return &tracker;
        }
    }

    return nullptr;
}

PipelineBuilder::PipelineBuilder (World *_targetWorld) noexcept
    : world (_targetWorld),
      registeredResources (world->pipelinePool.GetAllocationGroup ())
{
    assert (world);
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
    }

    currentPipelineId = _id;
    currentPipelineType = _type;

    currentPipelineAllocationGroup =
        Memory::Profiler::AllocationGroup {world->pipelinePool.GetAllocationGroup (), currentPipelineId};

    // taskRegister should be cleared in ::End.
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
    Pipeline *newPipeline = world->AddPipeline (currentPipelineId, currentPipelineType,
                                                taskRegister.ExportCollection (), _maximumChildThreads);
    taskRegister.Clear ();
    registeredResources.clear ();
    return newPipeline;
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
} // namespace Emergence::Celerity
