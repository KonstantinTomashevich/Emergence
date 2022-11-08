#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Events.hpp>
#include <Celerity/Render2d/RenderObject2dComponent.hpp>
#include <Celerity/Render2d/Rendering2d.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

namespace Emergence::Celerity::BoundsCalculation2d
{
const Memory::UniqueString Checkpoint::STARTED {"Render2dBoundsCalculationStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"Render2dBoundsCalculationFinished"};

class BoundsCalculator final : public TaskExecutorBase<BoundsCalculator>
{
public:
    BoundsCalculator (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void EnsureLocalBoundsExistence (UniqueId _objectId) noexcept;

    UniqueId AscendToTransformRoot (UniqueId _childObjectId) noexcept;

    UniqueId EnsureRenderObjectExistence (UniqueId _childObjectId) noexcept;

    void RequestLocalBoundsUpdate (UniqueId _objectId) noexcept;

    void RequestRenderObjectUpdate (UniqueId _objectId, bool _updateLocal) noexcept;

    void OnTransformParentChanged (UniqueId _objectId, UniqueId _oldParentId) noexcept;

    void OnLocalVisualTransformChanged (UniqueId _objectId) noexcept;

    void UpdateLocalBounds () noexcept;

    void UpdateRenderObjectBounds () noexcept;

    FetchSequenceQuery fetchSpriteAddedEvents;
    FetchSequenceQuery fetchSpriteSizeChangedEvents;
    FetchSequenceQuery fetchSpriteRemovedEvents;

    FetchSequenceQuery fetchTransformParentChangedEventsFixed;
    FetchSequenceQuery fetchTransformParentChangedEventsNormal;

    FetchSequenceQuery fetchVisualLocalTransformChangedEventsFixed;
    FetchSequenceQuery fetchVisualLocalTransformChangedEventsNormal;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    InsertLongTermQuery insertLocalBounds;
    ModifySignalQuery modifyDirtyLocalBounds;
    ModifyValueQuery modifyLocalBoundsByObjectId;
    FetchValueQuery fetchLocalBoundsByRenderObjectId;
    EditValueQuery editLocalBoundsByRenderObjectId;

    InsertLongTermQuery insertRenderObject;
    ModifySignalQuery modifyRenderObjectWithLocalDirty;
    EditSignalQuery editRenderObjectWithGlobalDirty;
    ModifyValueQuery modifyRenderObjectById;

    FetchValueQuery fetchSpriteByObjectId;
};

BoundsCalculator::BoundsCalculator (TaskConstructor &_constructor) noexcept
    : fetchSpriteAddedEvents (FETCH_SEQUENCE (Sprite2dAddedNormalEvent)),
      fetchSpriteSizeChangedEvents (FETCH_SEQUENCE (Sprite2dSizeChangedNormalEvent)),
      fetchSpriteRemovedEvents (FETCH_SEQUENCE (Sprite2dRemovedNormalEvent)),

      fetchTransformParentChangedEventsFixed (FETCH_SEQUENCE (Transform2dComponentParentChangedFixedToNormalEvent)),
      fetchTransformParentChangedEventsNormal (FETCH_SEQUENCE (Transform2dComponentParentChangedNormalEvent)),

      fetchVisualLocalTransformChangedEventsFixed (
          FETCH_SEQUENCE (Transform2dComponentVisualLocalTransformChangedFixedToNormalEvent)),
      fetchVisualLocalTransformChangedEventsNormal (
          FETCH_SEQUENCE (Transform2dComponentVisualLocalTransformChangedNormalEvent)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertLocalBounds (INSERT_LONG_TERM (LocalBounds2dComponent)),
      modifyDirtyLocalBounds (MODIFY_SIGNAL (LocalBounds2dComponent, dirty, true)),
      modifyLocalBoundsByObjectId (MODIFY_VALUE_1F (LocalBounds2dComponent, objectId)),
      fetchLocalBoundsByRenderObjectId (FETCH_VALUE_1F (LocalBounds2dComponent, renderObjectId)),
      editLocalBoundsByRenderObjectId (EDIT_VALUE_1F (LocalBounds2dComponent, renderObjectId)),

      insertRenderObject (INSERT_LONG_TERM (RenderObject2dComponent)),
      modifyRenderObjectWithLocalDirty (MODIFY_SIGNAL (RenderObject2dComponent, localDirty, true)),
      editRenderObjectWithGlobalDirty (EDIT_SIGNAL (RenderObject2dComponent, globalDirty, true)),
      modifyRenderObjectById (MODIFY_VALUE_1F (RenderObject2dComponent, objectId)),

      fetchSpriteByObjectId (FETCH_VALUE_1F (Sprite2dComponent, objectId))
{
    _constructor.DependOn (VisualTransformSync::Checkpoint::FINISHED);
    _constructor.DependOn (Rendering2d::Checkpoint::STARTED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Rendering2d::Checkpoint::FINISHED);
}

void BoundsCalculator::Execute () noexcept
{
    for (auto eventCursor = fetchSpriteAddedEvents.Execute ();
         const auto *event = static_cast<const Sprite2dAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        EnsureLocalBoundsExistence (event->objectId);
        RequestLocalBoundsUpdate (event->objectId);
    }

    for (auto eventCursor = fetchSpriteSizeChangedEvents.Execute ();
         const auto *event = static_cast<const Sprite2dSizeChangedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        RequestLocalBoundsUpdate (event->objectId);
    }

    for (auto eventCursor = fetchSpriteRemovedEvents.Execute ();
         const auto *event = static_cast<const Sprite2dRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        RequestLocalBoundsUpdate (event->objectId);
    }

    for (auto eventCursor = fetchTransformParentChangedEventsFixed.Execute ();
         const auto *event = static_cast<const Transform2dComponentParentChangedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        OnTransformParentChanged (event->objectId, event->oldParentId);
    }

    for (auto eventCursor = fetchTransformParentChangedEventsNormal.Execute ();
         const auto *event = static_cast<const Transform2dComponentParentChangedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        OnTransformParentChanged (event->objectId, event->oldParentId);
    }

    for (auto eventCursor = fetchVisualLocalTransformChangedEventsFixed.Execute ();
         const auto *event =
             static_cast<const Transform2dComponentVisualLocalTransformChangedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        OnLocalVisualTransformChanged (event->objectId);
    }

    for (auto eventCursor = fetchVisualLocalTransformChangedEventsNormal.Execute ();
         const auto *event =
             static_cast<const Transform2dComponentVisualLocalTransformChangedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        OnLocalVisualTransformChanged (event->objectId);
    }

    UpdateLocalBounds ();
    UpdateRenderObjectBounds ();
}

void BoundsCalculator::EnsureLocalBoundsExistence (UniqueId _objectId) noexcept
{
    {
        auto cursor = modifyLocalBoundsByObjectId.Execute (&_objectId);
        if (*cursor)
        {
            return;
        }
    }

    // Get the appropriate render object.
    UniqueId renderObjectId = EnsureRenderObjectExistence (_objectId);
    EMERGENCE_ASSERT (renderObjectId != INVALID_UNIQUE_ID);

    // Create local bounds as they do not exist.
    auto localBoundsCursor = insertLocalBounds.Execute ();
    auto *localBounds = static_cast<LocalBounds2dComponent *> (++localBoundsCursor);
    localBounds->objectId = _objectId;
    localBounds->renderObjectId = EnsureRenderObjectExistence (_objectId);
}

UniqueId BoundsCalculator::AscendToTransformRoot (UniqueId _childObjectId) noexcept
{
    UniqueId objectId = _childObjectId;
    while (true)
    {
        auto cursor = fetchTransformById.Execute (&objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*cursor);

        if (!transform)
        {
            // If transform hierarchy is broken, then there is no technical root.
            // It might happen for one frame when detached transform is getting removed.
            return INVALID_UNIQUE_ID;
        }

        if (transform->GetParentObjectId () == INVALID_UNIQUE_ID)
        {
            // We've reached the root.
            return objectId;
        }

        objectId = transform->GetParentObjectId ();
    }
}

UniqueId BoundsCalculator::EnsureRenderObjectExistence (UniqueId _childObjectId) noexcept
{
    const UniqueId rootId = AscendToTransformRoot (_childObjectId);
    EMERGENCE_ASSERT (rootId != INVALID_UNIQUE_ID);

    {
        auto cursor = modifyRenderObjectById.Execute (&rootId);
        if (*cursor)
        {
            return rootId;
        }
    }

    // Render object is not exists yet, we need to create it.
    auto renderObjectCursor = insertRenderObject.Execute ();
    auto *renderObject = static_cast<RenderObject2dComponent *> (++renderObjectCursor);
    renderObject->objectId = rootId;
    return rootId;
}

void BoundsCalculator::RequestLocalBoundsUpdate (UniqueId _objectId) noexcept
{
    if (auto cursor = modifyLocalBoundsByObjectId.Execute (&_objectId);
        auto *bounds = static_cast<LocalBounds2dComponent *> (*cursor))
    {
        bounds->dirty = true;
    }
}

void BoundsCalculator::RequestRenderObjectUpdate (UniqueId _objectId, bool _updateLocal) noexcept
{
    if (auto cursor = modifyRenderObjectById.Execute (&_objectId);
        auto *object = static_cast<RenderObject2dComponent *> (*cursor))
    {
        object->globalDirty = true;
        object->localDirty |= _updateLocal;
    }
}

void BoundsCalculator::OnTransformParentChanged (UniqueId _objectId, UniqueId _oldParentId) noexcept
{
    const UniqueId oldRootId = AscendToTransformRoot (_oldParentId);
    if (oldRootId == INVALID_UNIQUE_ID)
    {
        return;
    }

    {
        auto oldRenderObjectCursor = modifyRenderObjectById.Execute (&oldRootId);
        auto *oldRenderObject = static_cast<RenderObject2dComponent *> (*oldRenderObjectCursor);

        if (!oldRenderObject)
        {
            // Didn't belong to render-related hierarchy.
            return;
        }

        oldRenderObject->globalDirty = true;
        oldRenderObject->localDirty = true;
    }

    const UniqueId newRootId = EnsureRenderObjectExistence (_objectId);
    EMERGENCE_ASSERT (newRootId != INVALID_UNIQUE_ID);
    RequestRenderObjectUpdate (newRootId, true);

    for (auto cursor = editLocalBoundsByRenderObjectId.Execute (&oldRootId);
         auto *bounds = static_cast<LocalBounds2dComponent *> (*cursor); ++cursor)
    {
        const UniqueId trueRootId = AscendToTransformRoot (bounds->objectId);
        EMERGENCE_ASSERT (trueRootId == oldRootId || trueRootId == newRootId);
        bounds->renderObjectId = trueRootId;
    }
}

void BoundsCalculator::OnLocalVisualTransformChanged (UniqueId _objectId) noexcept
{
    const UniqueId rootId = AscendToTransformRoot (_objectId);
    if (rootId != INVALID_UNIQUE_ID)
    {
        RequestRenderObjectUpdate (rootId, _objectId != rootId);
    }
}

void BoundsCalculator::UpdateLocalBounds () noexcept
{
    for (auto localBoundsCursor = modifyDirtyLocalBounds.Execute ();
         auto *localBounds = static_cast<LocalBounds2dComponent *> (*localBoundsCursor);)
    {
        RequestRenderObjectUpdate (localBounds->renderObjectId, true);
        bool anyDrawableAttached = false;
        localBounds->bounds = {Math::Vector2f::ZERO, Math::Vector2f::ZERO};

        for (auto spriteCursor = fetchSpriteByObjectId.Execute (&localBounds->objectId);
             const auto *sprite = static_cast<const Sprite2dComponent *> (*spriteCursor); ++spriteCursor)
        {
            anyDrawableAttached = true;
            localBounds->bounds = Math::Combine (localBounds->bounds, {-sprite->halfSize, sprite->halfSize});
        }

        if (anyDrawableAttached)
        {
            localBounds->dirty = false;
            ++localBoundsCursor;
        }
        else
        {
            ~localBoundsCursor;
        }
    }
}

void BoundsCalculator::UpdateRenderObjectBounds () noexcept
{
    for (auto renderObjectCursor = modifyRenderObjectWithLocalDirty.Execute ();
         auto *renderObject = static_cast<RenderObject2dComponent *> (*renderObjectCursor);)
    {
        bool hasAnythingAttached = false;
        if (auto renderObjectTransformCursor = fetchTransformById.Execute (&renderObject->objectId);
            const auto *renderObjectTransform =
                static_cast<const Transform2dComponent *> (*renderObjectTransformCursor))
        {
            renderObject->local = {Math::Vector2f::ZERO, Math::Vector2f::ZERO};
            const Math::Matrix3x3f invertedRenderObjectLocalTransform =
                Math::Matrix3x3f {renderObjectTransform->GetVisualLocalTransform ()}.CalculateInverse ();

            for (auto localBoundsCursor = fetchLocalBoundsByRenderObjectId.Execute (&renderObject->objectId);
                 const auto *bounds = static_cast<const LocalBounds2dComponent *> (*localBoundsCursor);
                 ++localBoundsCursor)
            {
                if (auto boundsTransformCursor = fetchTransformById.Execute (&bounds->objectId);
                    const auto *boundsTransform = static_cast<const Transform2dComponent *> (*boundsTransformCursor))
                {
                    hasAnythingAttached = true;
                    const Math::Matrix3x3f worldTransformMatrix =
                        boundsTransform->GetVisualWorldTransform (transformWorldAccessor);

                    const Math::Matrix3x3f transformInRenderObjectSystem =
                        invertedRenderObjectLocalTransform * worldTransformMatrix;

                    const Math::AxisAlignedBox2d transformedBounds = transformInRenderObjectSystem * bounds->bounds;
                    renderObject->local = Math::Combine (renderObject->local, transformedBounds);
                }
            }
        }

        if (hasAnythingAttached)
        {
            renderObject->localDirty = false;
            ++renderObjectCursor;
        }
        else
        {
            ~renderObjectCursor;
        }
    }

    for (auto renderObjectCursor = editRenderObjectWithGlobalDirty.Execute ();
         auto *renderObject = static_cast<RenderObject2dComponent *> (*renderObjectCursor); ++renderObjectCursor)
    {
        if (auto renderObjectTransformCursor = fetchTransformById.Execute (&renderObject->objectId);
            const auto *renderObjectTransform =
                static_cast<const Transform2dComponent *> (*renderObjectTransformCursor))
        {
            const Math::Matrix3x3f renderObjectTransformMatrix {renderObjectTransform->GetVisualLocalTransform ()};
            renderObject->global = renderObjectTransformMatrix * renderObject->local;
            renderObject->globalDirty = false;
        }
    }
}

using namespace Memory::Literals;

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("CleanupRenderObject2dAfterTransformRemovalFromNormal"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, RenderObject2dComponent,
                                objectId)
        .MakeDependencyOf ("CleanupRenderObject2dAfterTransformRemovalFromFixed"_us);

    _pipelineBuilder.AddTask ("CleanupRenderObject2dAfterTransformRemovalFromFixed"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent,
                                RenderObject2dComponent, objectId)
        .MakeDependencyOf ("Render2dBoundsCalculator"_us);

    _pipelineBuilder.AddTask ("CleanupLocalBounds2dAfterTransformRemovalFromNormal"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, LocalBounds2dComponent,
                                objectId)
        .MakeDependencyOf ("CleanupLocalBounds2dAfterTransformRemovalFromFixed"_us);

    _pipelineBuilder.AddTask ("CleanupLocalBounds2dAfterTransformRemovalFromFixed"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent,
                                LocalBounds2dComponent, objectId)
        .MakeDependencyOf ("Render2dBoundsCalculator"_us);

    _pipelineBuilder.AddTask ("Render2dBoundsCalculator"_us).SetExecutor<BoundsCalculator> ();
}
} // namespace Emergence::Celerity::BoundsCalculation2d