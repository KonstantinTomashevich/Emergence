#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/FrameBufferManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/FrameBuffer.hpp>
#include <Celerity/Render/Foundation/FrameBufferTextureUsage.hpp>

namespace Emergence::Celerity::FrameBufferManagement
{
using namespace Memory::Literals;

class Manager final : public TaskExecutorBase<Manager>
{
public:
    Manager (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchAssetRemovedEvents;
    RemoveValueQuery removeFrameBufferById;
    RemoveValueQuery removeFrameBufferTextureUsageById;
};

Manager::Manager (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      removeFrameBufferById (REMOVE_VALUE_1F (FrameBuffer, assetId)),
      removeFrameBufferTextureUsageById (REMOVE_VALUE_1F (FrameBufferTextureUsage, assetId))
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

void Manager::Execute () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto frameBufferCursor = removeFrameBufferById.Execute (&event->id); frameBufferCursor.ReadConst ())
        {
            ~frameBufferCursor;
        }

        for (auto usageCursor = removeFrameBufferTextureUsageById.Execute (&event->id); usageCursor.ReadConst ();)
        {
            ~usageCursor;
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("FrameBufferManagement");
    _pipelineBuilder.AddTask ("FrameBufferManager"_us).SetExecutor<Manager> ();
}
} // namespace Emergence::Celerity::FrameBufferManagement
