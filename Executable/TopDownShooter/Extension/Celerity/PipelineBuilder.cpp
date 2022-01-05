#include <cassert>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
TaskConstructor::TaskConstructor (TaskConstructor &&_other) noexcept
    : parent (_other.parent),
      task (std::move (_other.task))
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

Warehouse::FetchSingletonQuery TaskConstructor::FetchSingleton (const StandardLayout::Mapping &_typeMapping)
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchSingleton (_typeMapping);
}

Warehouse::ModifySingletonQuery TaskConstructor::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifySingleton (_typeMapping);
}

Warehouse::InsertShortTermQuery TaskConstructor::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.InsertShortTerm (_typeMapping);
}

Warehouse::FetchSequenceQuery TaskConstructor::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchSequence (_typeMapping);
}

Warehouse::ModifySequenceQuery TaskConstructor::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifySequence (_typeMapping);
}

Warehouse::InsertLongTermQuery TaskConstructor::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.InsertLongTerm (_typeMapping);
}

Warehouse::FetchValueQuery TaskConstructor::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                                        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchValue (_typeMapping, _keyFields);
}

Warehouse::ModifyValueQuery TaskConstructor::ModifyValue (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifyValue (_typeMapping, _keyFields);
}

Warehouse::FetchAscendingRangeQuery TaskConstructor::FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                          StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchAscendingRange (_typeMapping, _keyField);
}

Warehouse::ModifyAscendingRangeQuery TaskConstructor::ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                            StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifyAscendingRange (_typeMapping, _keyField);
}

Warehouse::FetchDescendingRangeQuery TaskConstructor::FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                            StandardLayout::FieldId _keyField) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchDescendingRange (_typeMapping, _keyField);
}

Warehouse::ModifyDescendingRangeQuery TaskConstructor::ModifyDescendingRange (
    const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifyDescendingRange (_typeMapping, _keyField);
}

Warehouse::FetchShapeIntersectionQuery TaskConstructor::FetchShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchShapeIntersection (_typeMapping, _dimensions);
}

Warehouse::ModifyShapeIntersectionQuery TaskConstructor::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifyShapeIntersection (_typeMapping, _dimensions);
}

Warehouse::FetchRayIntersectionQuery TaskConstructor::FetchRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.readAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.FetchRayIntersection (_typeMapping, _dimensions);
}

Warehouse::ModifyRayIntersectionQuery TaskConstructor::ModifyRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Warehouse::Dimension> &_dimensions) noexcept
{
    task.writeAccess.emplace_back (_typeMapping.GetName ());
    return parent->world->registry.ModifyRayIntersection (_typeMapping, _dimensions);
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

TaskConstructor::TaskConstructor (PipelineBuilder *_parent, Memory::UniqueString _name) noexcept : parent (_parent)
{
    assert (parent);
    task.name = _name;
}

PipelineBuilder::PipelineBuilder (World *_targetWorld) noexcept : world (_targetWorld)
{
    assert (world);
}

void PipelineBuilder::Begin () noexcept
{
    // taskRegister should be cleared in ::End.
}

TaskConstructor PipelineBuilder::AddTask (Memory::UniqueString _name) noexcept
{
    return {this, _name};
}

void PipelineBuilder::AddCheckpoint (Memory::UniqueString _name) noexcept
{
    taskRegister.RegisterCheckpoint (_name);
}

Pipeline *PipelineBuilder::End (std::size_t _maximumChildThreads) noexcept
{
    auto &newPipeline =
        world->pipelines.emplace_back (new Pipeline (taskRegister.ExportCollection (), _maximumChildThreads));

    taskRegister.Clear ();
    registeredResources.clear ();
    return newPipeline.get ();
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
