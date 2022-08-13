#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/NonFeatureSpecificComponentCleanup.hpp>

namespace NonFeatureSpecificComponentCleanup
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"NonFeatureSpecificComponentCleanupStarted"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"NonFeatureSpecificComponentCleanupFinished"};

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("NonFutureSpecificComponentCleanup::AlignmentComponent"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, AlignmentComponent,
                                objectId)
        .DependOn (Checkpoint::STARTED)
        .MakeDependencyOf (Checkpoint::FINISHED)
        .MakeDependencyOf (Emergence::Celerity::Assembly::Checkpoint::STARTED);
}
} // namespace NonFeatureSpecificComponentCleanup
