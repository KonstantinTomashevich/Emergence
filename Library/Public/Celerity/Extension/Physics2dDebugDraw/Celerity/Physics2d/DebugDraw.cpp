#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/DebugDraw.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/Physics2dDebugDrawSingleton.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Physics2d/Simulation.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>

namespace Emergence::Celerity::Physics2dDebugDraw
{
const Memory::UniqueString Checkpoint::STARTED {"Physics2dDebugDraw::Started"};
const Memory::UniqueString Checkpoint::FINISHED {"Physics2dDebugDraw::Finished"};

static const Memory::UniqueString DETACHED_MATERIAL {"MI_Physics2dDebugDetached"};
static const Memory::UniqueString DYNAMIC_MATERIAL {"MI_Physics2dDebugDynamic"};
static const Memory::UniqueString KINEMATIC_MATERIAL {"MI_Physics2dDebugKinematic"};
static const Memory::UniqueString STATIC_MATERIAL {"MI_Physics2dDebugStatic"};

struct ShapeDebugDrawLink final
{
    UniqueId shapeId = INVALID_UNIQUE_ID;
    UniqueId debugShapeId = INVALID_UNIQUE_ID;

    struct Reflection final
    {
        StandardLayout::FieldId shapeId;
        StandardLayout::FieldId debugShapeId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const ShapeDebugDrawLink::Reflection &ShapeDebugDrawLink::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ShapeDebugDrawLink);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (debugShapeId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

class DebugDrawManager final : public TaskExecutorBase<DebugDrawManager>
{
public:
    DebugDrawManager (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void AddDebugDrawToAllShapes () noexcept;

    void RemoveDebugDrawFromAllShapes () noexcept;

    void AddShapeDebugDraw (UniqueId _shapeId) noexcept;

    void AddShapeDebugDraw (const CollisionShape2dComponent *_collisionShape) noexcept;

    void UpdateShapeDebugDraw (UniqueId _shapeId) noexcept;

    void UpdateShapeDebugDraw (const CollisionShape2dComponent *_collisionShape) noexcept;

    void UpdateShapeDebugDraw (const CollisionShape2dComponent *_collisionShape,
                               DebugShape2dComponent *_debugShape) noexcept;

    void RemoveShapeDebugDraw (UniqueId _shapeId) noexcept;

    ModifySingletonQuery modifyDebugDraw;
    FetchSingletonQuery fetchRender2d;

    FetchSequenceQuery fetchRigidBodyAddedFixedEvents;
    FetchSequenceQuery fetchRigidBodyAddedCustomEvents;
    FetchSequenceQuery fetchRigidBodyRemovedEvents;

    FetchSequenceQuery fetchCollisionShapeAddedFixedEvents;
    FetchSequenceQuery fetchCollisionShapeAddedCustomEvents;
    FetchSequenceQuery fetchCollisionShapeAttributesChangedEvents;
    FetchSequenceQuery fetchCollisionShapeGeometryChangedEvents;
    FetchSequenceQuery fetchCollisionShapeRemovedEvents;

    FetchAscendingRangeQuery fetchCollisionShapeByObjectIdAscending;
    FetchValueQuery fetchCollisionShapeByObjectId;
    FetchValueQuery fetchCollisionShapeByShapeId;

    InsertLongTermQuery insertShapeDebugDrawLink;
    ModifyAscendingRangeQuery modifyShapeDebugDrawLinkByShapeIdAscending;
    FetchValueQuery fetchShapeDebugDrawLinkByShapeId;
    ModifyValueQuery modifyShapeDebugDrawLinkByShapeId;

    InsertLongTermQuery insertDebugShape;
    ModifyValueQuery modifyDebugShapeByDebugShapeId;

    FetchValueQuery fetchRigidBodyByObjectId;
};

DebugDrawManager::DebugDrawManager (TaskConstructor &_constructor) noexcept
    : modifyDebugDraw (MODIFY_SINGLETON (Physics2dDebugDrawSingleton)),
      fetchRender2d (FETCH_SINGLETON (Render2dSingleton)),

      fetchRigidBodyAddedFixedEvents (FETCH_SEQUENCE (RigidBody2dComponentAddedFixedEvent)),
      fetchRigidBodyAddedCustomEvents (FETCH_SEQUENCE (RigidBody2dComponentAddedCustomToFixedEvent)),
      fetchRigidBodyRemovedEvents (FETCH_SEQUENCE (RigidBody2dComponentRemovedEvent)),

      fetchCollisionShapeAddedFixedEvents (FETCH_SEQUENCE (CollisionShape2dComponentAddedFixedEvent)),
      fetchCollisionShapeAddedCustomEvents (FETCH_SEQUENCE (CollisionShape2dComponentAddedCustomToFixedEvent)),
      fetchCollisionShapeAttributesChangedEvents (FETCH_SEQUENCE (CollisionShape2dComponentAttributesChangedEvent)),
      fetchCollisionShapeGeometryChangedEvents (FETCH_SEQUENCE (CollisionShape2dComponentGeometryChangedEvent)),
      fetchCollisionShapeRemovedEvents (FETCH_SEQUENCE (CollisionShape2dComponentRemovedEvent)),

      fetchCollisionShapeByObjectIdAscending (FETCH_ASCENDING_RANGE (CollisionShape2dComponent, objectId)),
      fetchCollisionShapeByObjectId (FETCH_VALUE_1F (CollisionShape2dComponent, objectId)),
      fetchCollisionShapeByShapeId (FETCH_VALUE_1F (CollisionShape2dComponent, shapeId)),

      insertShapeDebugDrawLink (INSERT_LONG_TERM (ShapeDebugDrawLink)),
      modifyShapeDebugDrawLinkByShapeIdAscending (MODIFY_ASCENDING_RANGE (ShapeDebugDrawLink, shapeId)),
      fetchShapeDebugDrawLinkByShapeId (FETCH_VALUE_1F (ShapeDebugDrawLink, shapeId)),
      modifyShapeDebugDrawLinkByShapeId (MODIFY_VALUE_1F (ShapeDebugDrawLink, shapeId)),

      insertDebugShape (INSERT_LONG_TERM (DebugShape2dComponent)),
      modifyDebugShapeByDebugShapeId (MODIFY_VALUE_1F (DebugShape2dComponent, debugShapeId)),

      fetchRigidBodyByObjectId (FETCH_VALUE_1F (RigidBody2dComponent, objectId))
{
    _constructor.DependOn (Physics2dSimulation::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void DebugDrawManager::Execute () noexcept
{
    auto debugDrawCursor = modifyDebugDraw.Execute ();
    auto *debugDraw = static_cast<Physics2dDebugDrawSingleton *> (*debugDrawCursor);

    if (debugDraw->wasEnabled != debugDraw->enabled)
    {
        if (debugDraw->enabled)
        {
            AddDebugDrawToAllShapes ();
        }
        else
        {
            RemoveDebugDrawFromAllShapes ();
        }

        debugDraw->wasEnabled = debugDraw->enabled;
    }
    else if (debugDraw->enabled)
    {
        for (auto eventCursor = fetchRigidBodyAddedFixedEvents.Execute ();
             const auto *event = static_cast<const RigidBody2dComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
        {
            for (auto shapeCursor = fetchCollisionShapeByObjectId.Execute (&event->objectId);
                 const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
            {
                UpdateShapeDebugDraw (shape);
            }
        }

        for (auto eventCursor = fetchRigidBodyAddedCustomEvents.Execute ();
             const auto *event = static_cast<const RigidBody2dComponentAddedCustomToFixedEvent *> (*eventCursor);
             ++eventCursor)
        {
            for (auto shapeCursor = fetchCollisionShapeByObjectId.Execute (&event->objectId);
                 const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
            {
                UpdateShapeDebugDraw (shape);
            }
        }

        for (auto eventCursor = fetchRigidBodyRemovedEvents.Execute ();
             const auto *event = static_cast<const RigidBody2dComponentRemovedEvent *> (*eventCursor); ++eventCursor)
        {
            for (auto shapeCursor = fetchCollisionShapeByObjectId.Execute (&event->objectId);
                 const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
            {
                UpdateShapeDebugDraw (shape);
            }
        }

        for (auto eventCursor = fetchCollisionShapeAddedFixedEvents.Execute ();
             const auto *event = static_cast<const CollisionShape2dComponentAddedFixedEvent *> (*eventCursor);
             ++eventCursor)
        {
            AddShapeDebugDraw (event->shapeId);
        }

        for (auto eventCursor = fetchCollisionShapeAddedCustomEvents.Execute ();
             const auto *event = static_cast<const CollisionShape2dComponentAddedCustomToFixedEvent *> (*eventCursor);
             ++eventCursor)
        {
            AddShapeDebugDraw (event->shapeId);
        }

        for (auto eventCursor = fetchCollisionShapeAttributesChangedEvents.Execute ();
             const auto *event = static_cast<const CollisionShape2dComponentAttributesChangedEvent *> (*eventCursor);
             ++eventCursor)
        {
            if (auto cursor = fetchCollisionShapeByShapeId.Execute (&event->shapeId);
                const auto *collisionShape = static_cast<const CollisionShape2dComponent *> (*cursor))
            {
                if (collisionShape->enabled)
                {
                    AddShapeDebugDraw (collisionShape);
                }
                else
                {
                    RemoveShapeDebugDraw (event->shapeId);
                }
            }
        }

        for (auto eventCursor = fetchCollisionShapeGeometryChangedEvents.Execute ();
             const auto *event = static_cast<const CollisionShape2dComponentGeometryChangedEvent *> (*eventCursor);
             ++eventCursor)
        {
            UpdateShapeDebugDraw (event->shapeId);
        }

        for (auto eventCursor = fetchCollisionShapeRemovedEvents.Execute ();
             const auto *event = static_cast<const CollisionShape2dComponentRemovedEvent *> (*eventCursor);
             ++eventCursor)
        {
            RemoveShapeDebugDraw (event->shapeId);
        }
    }
}

void DebugDrawManager::AddDebugDrawToAllShapes () noexcept
{
    for (auto cursor = fetchCollisionShapeByObjectIdAscending.Execute (nullptr, nullptr);
         const auto *collisionShape = static_cast<const CollisionShape2dComponent *> (*cursor); ++cursor)
    {
        AddShapeDebugDraw (collisionShape);
    }
}

void DebugDrawManager::RemoveDebugDrawFromAllShapes () noexcept
{
    for (auto cursor = modifyShapeDebugDrawLinkByShapeIdAscending.Execute (nullptr, nullptr);
         auto *link = static_cast<ShapeDebugDrawLink *> (*cursor); ~cursor)
    {
        if (auto shapeCursor = modifyDebugShapeByDebugShapeId.Execute (&link->debugShapeId); *shapeCursor)
        {
            ~shapeCursor;
        }
    }
}

void DebugDrawManager::AddShapeDebugDraw (UniqueId _shapeId) noexcept
{
    if (auto cursor = fetchCollisionShapeByShapeId.Execute (&_shapeId);
        const auto *collisionShape = static_cast<const CollisionShape2dComponent *> (*cursor))
    {
        AddShapeDebugDraw (collisionShape);
    }
}

void DebugDrawManager::AddShapeDebugDraw (const CollisionShape2dComponent *_collisionShape) noexcept
{
    if (!_collisionShape->enabled)
    {
        return;
    }

    {
        auto linkCursor = fetchShapeDebugDrawLinkByShapeId.Execute (&_collisionShape->shapeId);
        if (*linkCursor)
        {
            // Already linked, just update.
            UpdateShapeDebugDraw (_collisionShape);
            return;
        }
    }

    auto render2dCursor = fetchRender2d.Execute ();
    const auto *render2d = static_cast<const Render2dSingleton *> (*render2dCursor);

    auto linkCursor = insertShapeDebugDrawLink.Execute ();
    auto *link = static_cast<ShapeDebugDrawLink *> (++linkCursor);
    link->shapeId = _collisionShape->shapeId;
    link->debugShapeId = render2d->GenerateDebugShape2dId ();

    auto debugShapeCursor = insertDebugShape.Execute ();
    auto *debugShape = static_cast<DebugShape2dComponent *> (++debugShapeCursor);
    debugShape->objectId = _collisionShape->objectId;
    debugShape->debugShapeId = link->debugShapeId;
    UpdateShapeDebugDraw (_collisionShape, debugShape);
}

void DebugDrawManager::UpdateShapeDebugDraw (UniqueId _shapeId) noexcept
{
    if (auto cursor = fetchCollisionShapeByShapeId.Execute (&_shapeId);
        const auto *collisionShape = static_cast<const CollisionShape2dComponent *> (*cursor))
    {
        UpdateShapeDebugDraw (collisionShape);
    }
}

void DebugDrawManager::UpdateShapeDebugDraw (const CollisionShape2dComponent *_collisionShape) noexcept
{
    if (auto linkCursor = fetchShapeDebugDrawLinkByShapeId.Execute (&_collisionShape->shapeId);
        const auto *link = static_cast<const ShapeDebugDrawLink *> (*linkCursor))
    {
        auto debugShapeCursor = modifyDebugShapeByDebugShapeId.Execute (&link->debugShapeId);
        auto *debugShape = static_cast<DebugShape2dComponent *> (*debugShapeCursor);
        EMERGENCE_ASSERT (debugShape);
        UpdateShapeDebugDraw (_collisionShape, debugShape);
    }
}

void DebugDrawManager::UpdateShapeDebugDraw (const CollisionShape2dComponent *_collisionShape,
                                             DebugShape2dComponent *_debugShape) noexcept
{
    _debugShape->materialInstanceId = DETACHED_MATERIAL;
    if (auto cursor = fetchRigidBodyByObjectId.Execute (&_collisionShape->objectId);
        const auto *body = static_cast<const RigidBody2dComponent *> (*cursor))
    {
        switch (body->type)
        {
        case RigidBody2dType::STATIC:
            _debugShape->materialInstanceId = STATIC_MATERIAL;
            break;

        case RigidBody2dType::KINEMATIC:
            _debugShape->materialInstanceId = KINEMATIC_MATERIAL;
            break;

        case RigidBody2dType::DYNAMIC:
            _debugShape->materialInstanceId = DYNAMIC_MATERIAL;
            break;
        }
    }

    _debugShape->translation = _collisionShape->translation;
    _debugShape->rotation = _collisionShape->rotation;

    switch (_collisionShape->geometry.type)
    {
    case CollisionGeometry2dType::BOX:
        _debugShape->shape.type = DebugShape2dType::BOX;
        _debugShape->shape.boxHalfExtents = _collisionShape->geometry.boxHalfExtents;
        break;

    case CollisionGeometry2dType::CIRCLE:
        _debugShape->shape.type = DebugShape2dType::CIRCLE;
        _debugShape->shape.circleRadius = _collisionShape->geometry.circleRadius;
        break;

    case CollisionGeometry2dType::LINE:
        _debugShape->shape.type = DebugShape2dType::LINE;
        _debugShape->translation += _collisionShape->geometry.lineStart;
        _debugShape->shape.lineEnd = _collisionShape->geometry.lineEnd - _collisionShape->geometry.lineStart;
        break;
    }
}

void DebugDrawManager::RemoveShapeDebugDraw (UniqueId _shapeId) noexcept
{
    if (auto cursor = modifyShapeDebugDrawLinkByShapeId.Execute (&_shapeId);
        const auto *link = static_cast<ShapeDebugDrawLink *> (*cursor))
    {
        if (auto shapeCursor = modifyDebugShapeByDebugShapeId.Execute (&link->debugShapeId); *shapeCursor)
        {
            ~shapeCursor;
        }

        ~cursor;
    }
}

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Memory::UniqueString {"Physics2dDebugDrawManager"}).SetExecutor<DebugDrawManager> ();
}
} // namespace Emergence::Celerity::Physics2dDebugDraw
