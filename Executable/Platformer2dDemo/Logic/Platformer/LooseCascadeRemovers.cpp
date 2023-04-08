#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Platformer/Control/ControllableComponent.hpp>
#include <Platformer/LooseCascadeRemovers.hpp>
#include <Platformer/Team/TeamComponent.hpp>

namespace LooseCascadeRemovers
{
void AddToFixedPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;

    _pipelineBuilder.AddTask ("LooseCascadeRemovers::ControllableComponent"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, ControllableComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("LooseCascadeRemovers::TeamComponent"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, TeamComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
}
} // namespace LooseCascadeRemovers
