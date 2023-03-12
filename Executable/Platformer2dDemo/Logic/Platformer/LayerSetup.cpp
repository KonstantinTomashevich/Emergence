#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Platformer/LayerSetup.hpp>
#include <Platformer/LayerSetupComponent.hpp>

namespace LayerSetup
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"LayerSetup::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LayerSetup::Finished"};

class SpriteLayerInitializer final : public Emergence::Celerity::TaskExecutorBase<SpriteLayerInitializer>
{
public:
    SpriteLayerInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void TryFetchLayer (Emergence::Celerity::UniqueId _objectId, uint16_t &_layerOutput) noexcept;

    Emergence::Celerity::FetchSequenceQuery fetchSpriteAddedEvents;
    Emergence::Celerity::EditValueQuery editSpriteById;
    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Celerity::FetchValueQuery fetchLayerSetupById;
};

SpriteLayerInitializer::SpriteLayerInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchSpriteAddedEvents (FETCH_SEQUENCE (Emergence::Celerity::Sprite2dAddedNormalEvent)),
      editSpriteById (EDIT_VALUE_1F (Emergence::Celerity::Sprite2dComponent, spriteId)),
      fetchTransformById (FETCH_VALUE_1F (Emergence::Celerity::Transform2dComponent, objectId)),
      fetchLayerSetupById (FETCH_VALUE_1F (LayerSetupComponent, objectId))
{
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::TransformVisualSync::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
}

void SpriteLayerInitializer::Execute () noexcept
{
    for (auto eventCursor = fetchSpriteAddedEvents.Execute ();
         const auto *event = static_cast<const Emergence::Celerity::Sprite2dAddedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto spriteCursor = editSpriteById.Execute (&event->spriteId);
        if (auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (*spriteCursor))
        {
            TryFetchLayer (sprite->objectId, sprite->layer);
        }
    }
}

void SpriteLayerInitializer::TryFetchLayer (Emergence::Celerity::UniqueId _objectId, uint16_t &_layerOutput) noexcept
{
    auto layerSetupCursor = fetchLayerSetupById.Execute (&_objectId);
    if (const auto *layerSetup = static_cast<const LayerSetupComponent *> (*layerSetupCursor))
    {
        _layerOutput = layerSetup->layer;
        return;
    }

    auto transformCursor = fetchTransformById.Execute (&_objectId);
    const auto *transform = static_cast<const Emergence::Celerity::Transform2dComponent *> (*transformCursor);
    EMERGENCE_ASSERT (transform);

    if (transform->GetParentObjectId () != Emergence::Celerity::INVALID_UNIQUE_ID)
    {
        TryFetchLayer (transform->GetParentObjectId (), _layerOutput);
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"SpriteLayerInitializer"})
        .SetExecutor<SpriteLayerInitializer> ();
}
} // namespace LayerSetup
