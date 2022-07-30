#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/NonFeatureSpecificComponentCleanup.hpp>

#include <Shared/Checkpoint.hpp>

namespace NonFeatureSpecificComponentCleanup
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"NonFutureSpecificComponentCleanup::FixedUpdate"})
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, AlignmentComponent,
                                objectId)
        .DependOn (Checkpoint::NON_FEATURE_SPECIFIC_COMPONENT_CLEANUP_STARTED)
        .MakeDependencyOf (Checkpoint::NON_FEATURE_SPECIFIC_COMPONENT_CLEANUP_FINISHED)
        .MakeDependencyOf (Emergence::Celerity::Assembly::Checkpoint::ASSEMBLY_STARTED);
}
} // namespace NonFeatureSpecificComponentCleanup
