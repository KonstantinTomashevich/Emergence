#include <Celerity/UI/UI.hpp>
#include <Celerity/UI/UIHierarchyCleanup.hpp>
#include <Celerity/UI/UIProcessing.hpp>
#include <Celerity/UI/UIRendering.hpp>

namespace Emergence::Celerity::UI
{
const Memory::UniqueString Checkpoint::HIERARCHY_CLEANUP_STARTED {"UI::HierarchyCleanupStarted"};
const Memory::UniqueString Checkpoint::HIERARCHY_CLEANUP_FINISHED {"UI::HierarchyCleanupFinished"};

const Memory::UniqueString Checkpoint::UPDATE_STARTED {"UI::UpdateStarted"};
const Memory::UniqueString Checkpoint::UPDATE_FINISHED {"UI::UpdateFinished"};

const Memory::UniqueString Checkpoint::RENDER_STARTED {"UI::RenderStarted"};
const Memory::UniqueString Checkpoint::RENDER_FINISHED {"UI::RenderFinished"};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        InputStorage::FrameInputAccumulator *_inputAccumulator,
                        const InputStorage::KeyCodeMapping &_keyCodeMapping) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::HIERARCHY_CLEANUP_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::HIERARCHY_CLEANUP_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::UPDATE_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::UPDATE_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_FINISHED);

    UIHierarchyCleanup::AddToNormalUpdate (_pipelineBuilder);
    UIProcessing::AddToNormalUpdate (_pipelineBuilder, _inputAccumulator, _keyCodeMapping);
    UIRendering::AddToNormalUpdate (_pipelineBuilder);
}
} // namespace Emergence::Celerity::UI
