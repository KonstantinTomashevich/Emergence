#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Physics2d/Box2dAccessSingleton.hpp>
#include <Celerity/Physics2d/CollisionContact2d.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Physics2d/Simulation.hpp>
#include <Celerity/Physics2d/TriggerContact2d.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>
#include <Math/Transform2d.hpp>

BEGIN_MUTING_WARNINGS
#define B2_USER_SETTINGS
#include <box2d/box2d.h>
END_MUTING_WARNINGS

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Celerity::Physics2dSimulation
{
using namespace Memory::Literals;

namespace TaskNames
{
// TODO: Currently all tasks are sequential and are separated only to make event pipeline work properly.
//       It seems that parallelization is not possible, because of close logical coupling between bodies,
//       shapes and materials. Is there any way to make it better?

static const Memory::UniqueString UPDATE_WORLD {"Physics2d::UpdateWorld"};

static const Memory::UniqueString SYNC_MATERIAL_CHANGES {"Physics2d::SyncMaterialChanges"};
static const Memory::UniqueString APPLY_MATERIAL_DELETION {"Physics2d::ApplyMaterialDeletion"};

static const Memory::UniqueString INITIALIZE_SHAPES {"Physics2d::InitializeShapes"};
static const Memory::UniqueString SYNC_SHAPE_CHANGES {"Physics2d::SyncShapeChanges"};
static const Memory::UniqueString APPLY_SHAPE_DELETION {"Physics2d::ApplyShapeDeletion"};

static const Memory::UniqueString INITIALIZE_BODIES {"Physics2d::InitializeBodies"};
static const Memory::UniqueString APPLY_BODY_DELETION {"Physics2d::ApplyBodyDeletion"};

static const Memory::UniqueString SYNC_BODY_MASSES {"Physics2d::SyncBodyMasses"};
static const Memory::UniqueString EXECUTE_SIMULATION {"Physics2d::ExecuteSimulation"};
} // namespace TaskNames

// NOTE: Every task must register physics world singleton modify access if it works directly with Box2d objects.
//       It allows us to be sure that there is no multithreaded Box2d calls.

static void *Box2dProfiledAllocation (std::int32_t _size) noexcept;

static void Box2dProfiledFree (void *_memory) noexcept;

static void *Box2dEffectiveAllocation (std::int32_t _size) noexcept;

static void Box2dEffectiveFree (void *_memory) noexcept;

static void Box2dLog (const char *_information) noexcept;

static b2Vec2 ToBox2d (const Math::Vector2f &_vector) noexcept;

static Math::Vector2f FromBox2d (const b2Vec2 &_vector) noexcept;

static b2Shape::Type ToBox2d (CollisionGeometry2dType _type) noexcept;

static b2Filter ConstructBox2dFilter (uint8_t _collisionGroup,
                                      bool _maintainContactList,
                                      bool _visibleToWorldQueries) noexcept;

static uint8_t GetCollisionGroup (const b2Filter &_filter) noexcept;

static bool IsMaintainingContactList (const b2Filter &_filter) noexcept;

static bool IsVisibleToWorldQueries (const b2Filter &_filter) noexcept;

static void UpdateShapeMaterial (b2Fixture *_fixture, const DynamicsMaterial2d *_material) noexcept;

static bool UpdateShapeGeometryAndPose (const CollisionShape2dComponent *_shape,
                                        const Math::Vector2f &_worldScale) noexcept;

static void UpdateBoxGeometryAndPose (const CollisionShape2dComponent *_shape,
                                      const Math::Vector2f &_worldScale,
                                      b2PolygonShape *_polygonShape) noexcept;

static void UpdateCircleGeometryAndPose (const CollisionShape2dComponent *_shape,
                                         const Math::Vector2f &_worldScale,
                                         b2CircleShape *_circleShape) noexcept;

static void UpdateLineGeometryAndPose (const CollisionShape2dComponent *_shape,
                                       const Math::Vector2f &_worldScale,
                                       b2EdgeShape *_lineShape) noexcept;

static void ConstructBox2dShape (CollisionShape2dComponent *_shape,
                                 b2Body *_body,
                                 const DynamicsMaterial2d *_material,
                                 const Math::Vector2f &_worldScale) noexcept;

class WorldUpdater final : public TaskExecutorBase<WorldUpdater>
{
public:
    WorldUpdater (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static void EnsurePhysicsWorldReady (const PhysicsWorld2dSingleton *_physicsWorld) noexcept;

    void UpdateConfiguration (const PhysicsWorld2dSingleton *_physicsWorld) noexcept;

    static void UpdateBox2dWorld (const PhysicsWorld2dSingleton *_physicsWorld) noexcept;

    ModifySingletonQuery modifyBox2d;
    FetchSingletonQuery fetchPhysicsWorld;
    FetchSequenceQuery fetchConfigurationChangedEvents;
};

WorldUpdater::WorldUpdater (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld2dSingleton)),
      fetchConfigurationChangedEvents (FETCH_SEQUENCE (PhysicsWorld2dConfigurationChanged))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
}

void WorldUpdater::Execute () noexcept
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*physicsWorldCursor);
    EnsurePhysicsWorldReady (physicsWorld);
    UpdateConfiguration (physicsWorld);
}

void WorldUpdater::EnsurePhysicsWorldReady (const PhysicsWorld2dSingleton *_physicsWorld) noexcept
{
    auto &box2dWorld = block_cast<b2World *> (
        const_cast<decltype (PhysicsWorld2dSingleton::implementationBlock) &> (_physicsWorld->implementationBlock));

    if (!box2dWorld)
    {
        if (_physicsWorld->enableMemoryProfiling)
        {
            SetBox2dAllocators (Box2dProfiledAllocation, Box2dProfiledFree);
        }
        else
        {
            SetBox2dAllocators (Box2dEffectiveAllocation, Box2dEffectiveFree);
        }

        SetBox2dLogger (Box2dLog);
        box2dWorld = new (b2Alloc (sizeof (b2World))) b2World {ToBox2d (_physicsWorld->gravity)};
        UpdateBox2dWorld (_physicsWorld);
    }
}

void WorldUpdater::UpdateConfiguration (const PhysicsWorld2dSingleton *_physicsWorld) noexcept
{
    auto eventCursor = fetchConfigurationChangedEvents.Execute ();
    if (!*eventCursor)
    {
        return;
    }

    UpdateBox2dWorld (_physicsWorld);
}

void WorldUpdater::UpdateBox2dWorld (const PhysicsWorld2dSingleton *_physicsWorld) noexcept
{
    auto *box2dWorld = block_cast<b2World *> (_physicsWorld->implementationBlock);
    box2dWorld->SetGravity (ToBox2d (_physicsWorld->gravity));
}

class MaterialChangesSynchronizer final : public TaskExecutorBase<MaterialChangesSynchronizer>
{
public:
    MaterialChangesSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyBox2d;
    FetchValueQuery fetchMaterialById;
    FetchSequenceQuery fetchMaterialChangedEvents;
    FetchValueQuery fetchShapeByMaterialId;
    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

MaterialChangesSynchronizer::MaterialChangesSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial2d, id)),
      fetchMaterialChangedEvents (FETCH_SEQUENCE (DynamicsMaterial2dChangedEvent)),
      fetchShapeByMaterialId (FETCH_VALUE_1F (CollisionShape2dComponent, materialId)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody2dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::UPDATE_WORLD);
}

void MaterialChangesSynchronizer::Execute () noexcept
{
    auto massInvalidatedCursor = insertBodyMassInvalidatedEvents.Execute ();
    for (auto eventCursor = fetchMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial2dChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto materialCursor = fetchMaterialById.Execute (&event->id);
        if (const auto *material = static_cast<const DynamicsMaterial2d *> (*materialCursor))
        {
            for (auto shapeCursor = fetchShapeByMaterialId.Execute (&material->id);
                 const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
            {
                if (auto *fixture = static_cast<b2Fixture *> (shape->implementationHandle))
                {
                    UpdateShapeMaterial (fixture, material);
                    static_cast<RigidBody2dComponentMassInvalidatedEvent *> (++massInvalidatedCursor)->objectId =
                        shape->objectId;
                }
            }
        }
        else
        {
            // Material is already removed, event was outdated.
        }
    }
}

class MaterialDeleter final : public TaskExecutorBase<MaterialDeleter>
{
public:
    MaterialDeleter (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyBox2d;
    FetchSequenceQuery fetchMaterialRemovedEvents;
    RemoveValueQuery removeShapeByMaterialId;
};

MaterialDeleter::MaterialDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchMaterialRemovedEvents (FETCH_SEQUENCE (DynamicsMaterial2dRemovedEvent)),
      removeShapeByMaterialId (REMOVE_VALUE_1F (CollisionShape2dComponent, materialId))
{
    _constructor.DependOn (TaskNames::SYNC_MATERIAL_CHANGES);
}

void MaterialDeleter::Execute () noexcept
{
    for (auto eventCursor = fetchMaterialRemovedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial2dRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        for (auto shapeCursor = removeShapeByMaterialId.Execute (&event->id); shapeCursor.ReadConst ();)
        {
            // Shapes can not exist without material, therefore we are cleaning them up.
            ~shapeCursor;
        }
    }
}

class ShapeInitializer final : public TaskExecutorBase<ShapeInitializer>
{
public:
    ShapeInitializer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyBox2d;

    ModifyValueQuery modifyShapeByShapeId;
    FetchValueQuery fetchMaterialById;
    FetchValueQuery fetchBodyByObjectId;

    FetchValueQuery fetchTransformByObjectId;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchShapeAddedFixedEvents;
    FetchSequenceQuery fetchShapeAddedCustomEvents;
};

ShapeInitializer::ShapeInitializer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),

      modifyShapeByShapeId (MODIFY_VALUE_1F (CollisionShape2dComponent, shapeId)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial2d, id)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody2dComponent, objectId)),

      fetchTransformByObjectId (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeAddedFixedEvents (FETCH_SEQUENCE (CollisionShape2dComponentAddedFixedEvent)),
      fetchShapeAddedCustomEvents (FETCH_SEQUENCE (CollisionShape2dComponentAddedCustomToFixedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_MATERIAL_DELETION);
}

void ShapeInitializer::Execute ()
{
    auto initialize = [this] (UniqueId _shapeId)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&_shapeId);
        auto *shape = static_cast<CollisionShape2dComponent *> (*shapeCursor);

        if (!shape)
        {
            // Shape is already removed.
            return;
        }

        if (!shape->enabled)
        {
            // Shape is currently disabled.
            return;
        }

        auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
        const auto *material = static_cast<const DynamicsMaterial2d *> (*materialCursor);

        if (!material)
        {
            EMERGENCE_LOG (ERROR, "Physics2d: Unable to find DynamicsMaterial2d with id ", shape->materialId,
                           "! Shape, that attempts to use this material, will be deleted.");

            ~shapeCursor;
            return;
        }

        auto transformCursor = fetchTransformByObjectId.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics2d: Unable to add CollisionShape2dComponent to object with id ",
                           shape->objectId, ", because it has no Transform2dComponent!");

            ~shapeCursor;
            return;
        }

        const Math::Vector2f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        auto bodyCursor = fetchBodyByObjectId.Execute (&shape->objectId);

        if (const auto *body = static_cast<const RigidBody2dComponent *> (*bodyCursor))
        {
            if (auto *box2dBody = static_cast<b2Body *> (body->implementationHandle))
            {
                // We need to edit shape, because we're setting implementation handle. But we cannot register edition
                // as it would trigger possibility of edition events for event validation pipeline, which would trigger
                // an error, because these events are processed right now.
                ConstructBox2dShape (shape, box2dBody, material, worldScale);
                // Body mass is automatically calculated, therefore there is no need to invalidate it.
            }
        }
    };

    for (auto eventCursor = fetchShapeAddedFixedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentAddedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->shapeId);
    }

    for (auto eventCursor = fetchShapeAddedCustomEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->shapeId);
    }
}

struct ShapeRemoveHelper
{
public:
    ShapeRemoveHelper (TaskConstructor &_constructor) noexcept;

protected:
    void RemoveShape (UniqueId _objectId, UniqueId _shapeId, void *_implementationHandle) noexcept;

    FetchValueQuery fetchBodyByObjectId;
    RemoveValueQuery removeCollisionContactByShapeId;
    RemoveValueQuery removeCollisionContactByOtherShapeId;
    RemoveValueQuery removeTriggerContactByTriggerShapeId;
    RemoveValueQuery removeTriggerContactByIntruderShapeId;
};

ShapeRemoveHelper::ShapeRemoveHelper (TaskConstructor &_constructor) noexcept
    : fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody2dComponent, objectId)),
      removeCollisionContactByShapeId (REMOVE_VALUE_1F (CollisionContact2d, shapeId)),
      removeCollisionContactByOtherShapeId (REMOVE_VALUE_1F (CollisionContact2d, otherShapeId)),
      removeTriggerContactByTriggerShapeId (REMOVE_VALUE_1F (TriggerContact2d, triggerShapeId)),
      removeTriggerContactByIntruderShapeId (REMOVE_VALUE_1F (TriggerContact2d, intruderShapeId))
{
}

void ShapeRemoveHelper::RemoveShape (UniqueId _objectId, UniqueId _shapeId, void *_implementationHandle) noexcept
{
    if (auto *fixture = static_cast<b2Fixture *> (_implementationHandle))
    {
        auto bodyCursor = fetchBodyByObjectId.Execute (&_objectId);
        if (const auto *body = static_cast<const RigidBody2dComponent *> (*bodyCursor);
            body && body->implementationHandle)
        {
            static_cast<b2Body *> (body->implementationHandle)->DestroyFixture (fixture);
            // Do not need to invalidate mass as it is invalidated automatically.
        }
        else
        {
            // Body was removed too.
        }
    }

    // Because shape is deleted, we need to delete all contacts too.
    for (auto cursor = removeCollisionContactByShapeId.Execute (&_shapeId); cursor.ReadConst ();)
    {
        ~cursor;
    }

    for (auto cursor = removeCollisionContactByOtherShapeId.Execute (&_shapeId); cursor.ReadConst ();)
    {
        ~cursor;
    }

    for (auto cursor = removeTriggerContactByTriggerShapeId.Execute (&_shapeId); cursor.ReadConst ();)
    {
        ~cursor;
    }

    for (auto cursor = removeTriggerContactByIntruderShapeId.Execute (&_shapeId); cursor.ReadConst ();)
    {
        ~cursor;
    }
}

class ShapeChangesSynchronizer final : public TaskExecutorBase<ShapeChangesSynchronizer>, public ShapeRemoveHelper
{
public:
    ShapeChangesSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void ApplyShapeMaterialChanges () noexcept;

    void ApplyShapeGeometryChanges () noexcept;

    void ApplyShapeAttributesChanges () noexcept;

    ModifySingletonQuery modifyBox2d;

    FetchValueQuery fetchShapeByShapeId;
    RemoveValueQuery removeShapeByShapeId;
    FetchValueQuery fetchMaterialById;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchShapeMaterialChangedEvents;
    FetchSequenceQuery fetchShapeGeometryChangedEvents;
    FetchSequenceQuery fetchShapeAttributesChangedEvents;

    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeChangesSynchronizer::ShapeChangesSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),
      ShapeRemoveHelper (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchShapeByShapeId (FETCH_VALUE_1F (CollisionShape2dComponent, shapeId)),
      removeShapeByShapeId (REMOVE_VALUE_1F (CollisionShape2dComponent, shapeId)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial2d, id)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeMaterialChangedEvents (FETCH_SEQUENCE (CollisionShape2dComponentMaterialChangedEvent)),
      fetchShapeGeometryChangedEvents (FETCH_SEQUENCE (CollisionShape2dComponentGeometryChangedEvent)),
      fetchShapeAttributesChangedEvents (FETCH_SEQUENCE (CollisionShape2dComponentAttributesChangedEvent)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody2dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_SHAPES);
}

void ShapeChangesSynchronizer::Execute ()
{
    ApplyShapeMaterialChanges ();
    ApplyShapeAttributesChanges ();
    ApplyShapeGeometryChanges ();
}

void ShapeChangesSynchronizer::ApplyShapeMaterialChanges () noexcept
{
    for (auto eventCursor = fetchShapeMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentMaterialChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShape2dComponent *> (shapeCursor.ReadConst ()))
        {
            if (shape->implementationHandle)
            {
                auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
                if (const auto *material = static_cast<const DynamicsMaterial2d *> (*materialCursor))
                {
                    UpdateShapeMaterial (static_cast<b2Fixture *> (shape->implementationHandle), material);
                    auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                    static_cast<RigidBody2dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
                }
                else
                {
                    EMERGENCE_LOG (ERROR, "Physics2d: Unable to find DynamicsMaterial2d with id ", shape->materialId,
                                   "! Shape, that attempts to use this material, will be deleted.");

                    ~shapeCursor;
                    continue;
                }
            }
            else
            {
                // Shape is not initialized yet.
            }
        }
    }
}

void ShapeChangesSynchronizer::ApplyShapeGeometryChanges () noexcept
{
    for (auto eventCursor = fetchShapeGeometryChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentGeometryChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        const auto *shape = static_cast<const CollisionShape2dComponent *> (shapeCursor.ReadConst ());

        if (!shape)
        {
            continue;
        }

        auto transformCursor = fetchTransformById.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics2d: Unable to update CollisionShape2dComponent to object with id ",
                           shape->objectId, ", because it has no Transform2dComponent!");

            ~shapeCursor;
            continue;
        }

        const Math::Vector2f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        if (!UpdateShapeGeometryAndPose (shape, worldScale))
        {
            // Shape type was changed, therefore shape can not be correctly represented anymore.
            ~shapeCursor;
            continue;
        }

        auto cursor = insertBodyMassInvalidatedEvents.Execute ();
        static_cast<RigidBody2dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
    }
}

void ShapeChangesSynchronizer::ApplyShapeAttributesChanges () noexcept
{
    for (auto eventCursor = fetchShapeAttributesChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentAttributesChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = fetchShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor))
        {
            if (shape->enabled && !shape->implementationHandle)
            {
                auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
                const auto *material = static_cast<const DynamicsMaterial2d *> (*materialCursor);

                if (!material)
                {
                    EMERGENCE_LOG (ERROR, "Physics2d: Unable to find DynamicsMaterial2d with id ", shape->materialId,
                                   "!");
                    continue;
                }

                auto transformCursor = fetchTransformById.Execute (&shape->objectId);
                const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

                if (!transform)
                {
                    EMERGENCE_LOG (
                        ERROR, "Physics2d: Unable to add backend shape of CollisionShape2dComponent to object with id ",
                        shape->objectId, ", because it has no Transform2dComponent!");
                    return;
                }

                const Math::Vector2f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
                auto bodyCursor = fetchBodyByObjectId.Execute (&shape->objectId);

                if (const auto *body = static_cast<const RigidBody2dComponent *> (*bodyCursor))
                {
                    if (auto *box2dBody = static_cast<b2Body *> (body->implementationHandle))
                    {
                        ConstructBox2dShape (const_cast<CollisionShape2dComponent *> (shape), box2dBody, material,
                                             worldScale);
                        // Body mass is automatically calculated, therefore there is no need to invalidate it.
                    }
                }
            }
            else if (!shape->enabled && shape->implementationHandle)
            {
                RemoveShape (shape->objectId, shape->shapeId, shape->implementationHandle);
                const_cast<CollisionShape2dComponent *> (shape)->implementationHandle = nullptr;
            }
            else if (auto *fixture = static_cast<b2Fixture *> (shape->implementationHandle))
            {
                fixture->SetSensor (shape->trigger);
                fixture->SetFilterData (ConstructBox2dFilter (shape->collisionGroup, shape->maintainCollisionContacts,
                                                              shape->visibleToWorldQueries));
            }
        }
    }
}

class ShapeDeleter final : public TaskExecutorBase<ShapeDeleter>, public ShapeRemoveHelper
{
public:
    ShapeDeleter (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyBox2d;
    FetchValueQuery fetchBodyByObjectId;
    FetchSequenceQuery fetchShapeRemovedEvents;

    RemoveValueQuery removeCollisionContactByShapeId;
    RemoveValueQuery removeCollisionContactByOtherShapeId;
    RemoveValueQuery removeTriggerContactByTriggerShapeId;
    RemoveValueQuery removeTriggerContactByIntruderShapeId;
};

ShapeDeleter::ShapeDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),
      ShapeRemoveHelper (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody2dComponent, objectId)),
      fetchShapeRemovedEvents (FETCH_SEQUENCE (CollisionShape2dComponentRemovedEvent)),

      removeCollisionContactByShapeId (REMOVE_VALUE_1F (CollisionContact2d, shapeId)),
      removeCollisionContactByOtherShapeId (REMOVE_VALUE_1F (CollisionContact2d, otherShapeId)),
      removeTriggerContactByTriggerShapeId (REMOVE_VALUE_1F (TriggerContact2d, triggerShapeId)),
      removeTriggerContactByIntruderShapeId (REMOVE_VALUE_1F (TriggerContact2d, intruderShapeId))
{
    _constructor.DependOn (TaskNames::SYNC_SHAPE_CHANGES);
}

void ShapeDeleter::Execute ()
{
    for (auto eventCursor = fetchShapeRemovedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape2dComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        RemoveShape (event->objectId, event->shapeId, event->implementationHandle);
    }
}

class BodyInitializer final : public TaskExecutorBase<BodyInitializer>
{
public:
    BodyInitializer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyBox2d;
    FetchSingletonQuery fetchPhysicsWorld;

    FetchValueQuery fetchMaterialById;
    ModifyValueQuery modifyBodyByObjectId;
    FetchValueQuery fetchShapeByObjectId;
    FetchValueQuery fetchTransformByObjectId;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchBodyAddedFixedEvents;
    FetchSequenceQuery fetchBodyAddedCustomEvents;
};

BodyInitializer::BodyInitializer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld2dSingleton)),

      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial2d, id)),
      modifyBodyByObjectId (MODIFY_VALUE_1F (RigidBody2dComponent, objectId)),
      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape2dComponent, objectId)),
      fetchTransformByObjectId (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchBodyAddedFixedEvents (FETCH_SEQUENCE (RigidBody2dComponentAddedFixedEvent)),
      fetchBodyAddedCustomEvents (FETCH_SEQUENCE (RigidBody2dComponentAddedCustomToFixedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_SHAPE_DELETION);
}

void BodyInitializer::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*physicsWorldCursor);
    auto *box2dWorld = block_cast<b2World *> (physicsWorld->implementationBlock);

    auto initialize = [this, &box2dWorld] (UniqueId _objectId)
    {
        auto bodyCursor = modifyBodyByObjectId.Execute (&_objectId);
        auto *body = static_cast<RigidBody2dComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed.
            return;
        }

        auto transformCursor = fetchTransformByObjectId.Execute (&_objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics2d: Unable to initialize RigidBody2dComponent on object with id ",
                           body->objectId, ", because it has no Transform2dComponent!");

            ~bodyCursor;
            return;
        }

        const Math::Transform2d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        b2BodyDef bodyDefinition;
        bodyDefinition.position = ToBox2d (logicalTransform.translation);
        bodyDefinition.angle = logicalTransform.rotation;
        bodyDefinition.userData.objectId = body->objectId;

        switch (body->type)
        {
        case RigidBody2dType::STATIC:
        {
            bodyDefinition.type = b2BodyType::b2_staticBody;
            break;
        }

        case RigidBody2dType::KINEMATIC:
        {
            bodyDefinition.type = b2BodyType::b2_kinematicBody;
            bodyDefinition.bullet = body->continuousCollisionDetection;
            bodyDefinition.linearVelocity = ToBox2d (body->linearVelocity);
            bodyDefinition.angularVelocity = body->angularVelocity;
            break;
        }

        case RigidBody2dType::DYNAMIC:
        {
            bodyDefinition.type = b2BodyType::b2_dynamicBody;
            bodyDefinition.linearDamping = body->linearDamping;
            bodyDefinition.angularDamping = body->angularDamping;
            bodyDefinition.bullet = body->continuousCollisionDetection;

            bodyDefinition.linearVelocity = ToBox2d (body->linearVelocity);
            bodyDefinition.angularVelocity = body->angularVelocity;
            bodyDefinition.fixedRotation = body->fixedRotation;
            bodyDefinition.allowSleep = true;

            if (body->affectedByGravity)
            {
                bodyDefinition.gravityScale = 1.0f;
            }
            else
            {
                bodyDefinition.gravityScale = 0.0f;
            }

            break;
        }
        }

        b2Body *box2dBody = box2dWorld->CreateBody (&bodyDefinition);
        body->implementationHandle = box2dBody;

        for (auto shapeCursor = fetchShapeByObjectId.Execute (&body->objectId);
             const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
        {
            if (!shape->enabled)
            {
                continue;
            }

            // If shapes are initialized, then cleanup after body removal
            // wasn't executed or there is another body on this object.
            EMERGENCE_ASSERT (!shape->implementationHandle);

            if (auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
                const auto *material = static_cast<const DynamicsMaterial2d *> (*materialCursor))
            {
                // We need to edit shape, because we're setting implementation handle. But we cannot register edition
                // as it would trigger possibility of edition events for event validation pipeline, which would trigger
                // an error, because these events are processed earlier.
                ConstructBox2dShape (const_cast<CollisionShape2dComponent *> (shape), box2dBody, material,
                                     logicalTransform.scale);
            }
            else
            {
                // Should never happen, because materialless shapes are cleared by prior tasks.
                EMERGENCE_ASSERT (false);
            }
        }

        if (body->type == RigidBody2dType::DYNAMIC)
        {
            if (!Math::NearlyEqual (body->additiveLinearImpulse, Math::Vector2f::ZERO))
            {
                box2dBody->ApplyLinearImpulseToCenter (ToBox2d (body->additiveLinearImpulse), true);
            }

            if (!Math::NearlyEqual (body->additiveAngularImpulse, 0.0f))
            {
                box2dBody->ApplyAngularImpulse (body->additiveAngularImpulse, true);
            }
        }

        // Body mass is automatically calculated, therefore there is no need to invalidate it.
    };

    for (auto eventCursor = fetchBodyAddedFixedEvents.Execute ();
         const auto *event = static_cast<const RigidBody2dComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->objectId);
    }

    for (auto eventCursor = fetchBodyAddedCustomEvents.Execute ();
         const auto *event = static_cast<const RigidBody2dComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->objectId);
    }
}

class BodyDeleter final : public TaskExecutorBase<BodyDeleter>
{
public:
    BodyDeleter (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyBox2d;
    FetchValueQuery fetchShapeByObjectId;
    FetchSequenceQuery fetchBodyRemovedEvents;

    RemoveValueQuery removeCollisionContactByObjectId;
    RemoveValueQuery removeCollisionContactByOtherObjectId;
    RemoveValueQuery removeTriggerContactByTriggerObjectId;
    RemoveValueQuery removeTriggerContactByIntruderObjectId;
};

BodyDeleter::BodyDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape2dComponent, objectId)),
      fetchBodyRemovedEvents (FETCH_SEQUENCE (RigidBody2dComponentRemovedEvent)),

      removeCollisionContactByObjectId (REMOVE_VALUE_1F (CollisionContact2d, objectId)),
      removeCollisionContactByOtherObjectId (REMOVE_VALUE_1F (CollisionContact2d, otherObjectId)),
      removeTriggerContactByTriggerObjectId (REMOVE_VALUE_1F (TriggerContact2d, triggerObjectId)),
      removeTriggerContactByIntruderObjectId (REMOVE_VALUE_1F (TriggerContact2d, intruderObjectId))
{
    _constructor.DependOn (TaskNames::INITIALIZE_BODIES);
}

void BodyDeleter::Execute ()
{
    for (auto eventCursor = fetchBodyRemovedEvents.Execute ();
         const auto *event = static_cast<const RigidBody2dComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        for (auto shapeCursor = fetchShapeByObjectId.Execute (&event->objectId);
             const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
        {
            // Body deletes all its shapes. We cannot edit shape and need const cast to change implementation handle,
            // because edition will trigger event pipeline validation and trigger error as shape edition is processed
            // earlier.
            const_cast<CollisionShape2dComponent *> (shape)->implementationHandle = nullptr;
        }

        // Because all shapes are deleted, we need to delete all contacts too.
        for (auto cursor = removeCollisionContactByObjectId.Execute (&event->objectId); cursor.ReadConst ();)
        {
            ~cursor;
        }

        for (auto cursor = removeCollisionContactByOtherObjectId.Execute (&event->objectId); cursor.ReadConst ();)
        {
            ~cursor;
        }

        for (auto cursor = removeTriggerContactByTriggerObjectId.Execute (&event->objectId); cursor.ReadConst ();)
        {
            ~cursor;
        }

        for (auto cursor = removeTriggerContactByIntruderObjectId.Execute (&event->objectId); cursor.ReadConst ();)
        {
            ~cursor;
        }
    }
}

class BodyMassSynchronizer final : public TaskExecutorBase<BodyMassSynchronizer>
{
public:
    BodyMassSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyBox2d;
    FetchValueQuery fetchBodyByObjectId;
    FetchSequenceQuery fetchBodyMassInvalidationEvents;
};

BodyMassSynchronizer::BodyMassSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody2dComponent, objectId)),
      fetchBodyMassInvalidationEvents (FETCH_SEQUENCE (RigidBody2dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_BODY_DELETION);
}

void BodyMassSynchronizer::Execute ()
{
    // Body mass recalculation is a rare task, therefore we can afford to allocate memory dynamically.
    Container::Vector<UniqueId> bodyIds {heap.GetAllocationGroup ()};

    // Filter out duplicates to avoid excessive mass recalculations (they're quite expensive).
    for (auto eventCursor = fetchBodyMassInvalidationEvents.Execute ();
         const auto *event = static_cast<const RigidBody2dComponentMassInvalidatedEvent *> (*eventCursor);
         ++eventCursor)
    {
        Container::AddUnique (bodyIds, event->objectId);
    }

    for (UniqueId objectId : bodyIds)
    {
        auto bodyCursor = fetchBodyByObjectId.Execute (&objectId);
        const auto *body = static_cast<const RigidBody2dComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed, no need to bother with recalculation.
            continue;
        }

        if (body->type != RigidBody2dType::DYNAMIC)
        {
            // Static bodies have no mass.
            continue;
        }

        static_cast<b2Body *> (body->implementationHandle)->ResetMassData ();
    }
}

class SimulationExecutor final : public TaskExecutorBase<SimulationExecutor>,
                                 public b2ContactFilter,
                                 public b2ContactListener
{
public:
    SimulationExecutor (TaskConstructor &_constructor) noexcept;

    void Execute ();

    bool ShouldCollide (b2Fixture *_fixtureA, b2Fixture *_fixtureB) noexcept override;

    void BeginContact (b2Contact *_contact) noexcept override;

    void PostSolve (b2Contact *_contact, [[maybe_unused]] const b2ContactImpulse *_impulse) noexcept override;

    void EndContact (b2Contact *_contact) noexcept override;

private:
    void SyncBodiesWithOutsideManipulations () noexcept;

    void ExecuteSimulation (const PhysicsWorld2dSingleton *_physicsWorld, float _timeStep) noexcept;

    void SyncKinematicAndDynamicBodies () noexcept;

    ModifySingletonQuery modifyBox2d;
    FetchSingletonQuery fetchPhysicsWorld;
    FetchSingletonQuery fetchTime;

    FetchValueQuery fetchShapeByObjectId;
    EditSignalQuery editBodyWithOutsideManipulations;
    EditSignalQuery editKinematicBody;
    EditSignalQuery editDynamicBody;

    FetchValueQuery fetchTransformByObjectId;
    EditValueQuery editTransformByObjectId;
    Transform2dWorldAccessor transformWorldAccessor;

    InsertLongTermQuery insertCollisionContact;
    ModifyValueQuery modifyCollisionContactByShapeIdAndOtherShapeId;

    InsertLongTermQuery insertTriggerContact;
    ModifyValueQuery modifyTriggerContactByTriggerShapeIdAndIntruderShapeId;
};

SimulationExecutor::SimulationExecutor (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBox2d (MODIFY_SINGLETON (Box2dAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld2dSingleton)),
      fetchTime (FETCH_SINGLETON (TimeSingleton)),

      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape2dComponent, objectId)),
      editBodyWithOutsideManipulations (EDIT_SIGNAL (RigidBody2dComponent, manipulatedOutsideOfSimulation, true)),
      editKinematicBody (EDIT_SIGNAL (RigidBody2dComponent, type, RigidBody2dType::KINEMATIC)),
      editDynamicBody (EDIT_SIGNAL (RigidBody2dComponent, type, RigidBody2dType::DYNAMIC)),

      fetchTransformByObjectId (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      editTransformByObjectId (EDIT_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertCollisionContact (INSERT_LONG_TERM (CollisionContact2d)),
      modifyCollisionContactByShapeIdAndOtherShapeId (MODIFY_VALUE_2F (CollisionContact2d, shapeId, otherShapeId)),

      insertTriggerContact (INSERT_LONG_TERM (TriggerContact2d)),
      modifyTriggerContactByTriggerShapeIdAndIntruderShapeId (
          MODIFY_VALUE_2F (TriggerContact2d, triggerShapeId, intruderShapeId))
{
    _constructor.DependOn (TaskNames::SYNC_BODY_MASSES);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void SimulationExecutor::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*physicsWorldCursor);

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    SyncBodiesWithOutsideManipulations ();
    ExecuteSimulation (physicsWorld, time->fixedDurationS);
    SyncKinematicAndDynamicBodies ();
}

bool SimulationExecutor::ShouldCollide (b2Fixture *_fixtureA, b2Fixture *_fixtureB) noexcept
{
    const auto &worldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*worldCursor);

    const bool collision0to1 = physicsWorld->collisionMasks[GetCollisionGroup (_fixtureA->GetFilterData ())] &
                               (1u << GetCollisionGroup (_fixtureB->GetFilterData ()));

    const bool collision1to0 = physicsWorld->collisionMasks[GetCollisionGroup (_fixtureB->GetFilterData ())] &
                               (1u << GetCollisionGroup (_fixtureA->GetFilterData ()));

    return collision0to1 || collision1to0;
}

void SimulationExecutor::BeginContact (b2Contact *_contact) noexcept
{
    const b2Fixture *firstShape = _contact->GetFixtureA ();
    const b2Fixture *secondShape = _contact->GetFixtureB ();

    const b2Body *firstBody = firstShape->GetBody ();
    const b2Body *secondBody = secondShape->GetBody ();

    const bool firstSensor = firstShape->IsSensor ();
    const bool secondSensor = secondShape->IsSensor ();
    EMERGENCE_ASSERT (!firstSensor || !secondSensor);

    if (firstSensor || secondSensor)
    {
        auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
        const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*physicsWorldCursor);

        auto cursor = insertTriggerContact.Execute ();
        auto *contact = static_cast<TriggerContact2d *> (++cursor);
        contact->triggerContactId = physicsWorld->GenerateTriggerContactId ();

        if (firstSensor)
        {
            contact->triggerObjectId = firstBody->GetUserData ().objectId;
            contact->triggerShapeId = firstShape->GetUserData ().shapeId;
            contact->intruderObjectId = secondBody->GetUserData ().objectId;
            contact->intruderShapeId = secondShape->GetUserData ().shapeId;
        }
        else
        {
            contact->triggerObjectId = secondBody->GetUserData ().objectId;
            contact->triggerShapeId = secondShape->GetUserData ().shapeId;
            contact->intruderObjectId = firstBody->GetUserData ().objectId;
            contact->intruderShapeId = firstShape->GetUserData ().shapeId;
        }
    }
    else
    {
        const bool addCollisionContactToFirst = IsMaintainingContactList (firstShape->GetFilterData ());
        const bool addCollisionContactToSecond = IsMaintainingContactList (secondShape->GetFilterData ());

        if (addCollisionContactToFirst || addCollisionContactToSecond)
        {
            b2WorldManifold contactWorldManifold;
            _contact->GetWorldManifold (&contactWorldManifold);

            auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
            const auto *physicsWorld = static_cast<const PhysicsWorld2dSingleton *> (*physicsWorldCursor);
            auto cursor = insertCollisionContact.Execute ();

            auto addContact = [&cursor, physicsWorld, _contact, &contactWorldManifold] (
                                  const b2Body *_body, const b2Fixture *_shape, const b2Body *_otherBody,
                                  const b2Fixture *_otherShape)
            {
                auto *contact = static_cast<CollisionContact2d *> (++cursor);
                contact->collisionContactId = physicsWorld->GenerateCollisionContactId ();
                contact->objectId = _body->GetUserData ().objectId;
                contact->shapeId = _shape->GetUserData ().shapeId;
                contact->otherObjectId = _otherBody->GetUserData ().objectId;
                contact->otherShapeId = _otherShape->GetUserData ().shapeId;
                contact->normal = FromBox2d (contactWorldManifold.normal);

                for (size_t pointIndex = 0u; pointIndex < static_cast<size_t> (_contact->GetManifold ()->pointCount);
                     ++pointIndex)
                {
                    [[maybe_unused]] const bool emplaceSuccessful =
                        contact->points.TryEmplaceBack (FromBox2d (contactWorldManifold.points[pointIndex]));
                    EMERGENCE_ASSERT (emplaceSuccessful);
                }
            };

            if (addCollisionContactToFirst)
            {
                addContact (firstBody, firstShape, secondBody, secondShape);
            }

            if (addCollisionContactToSecond)
            {
                addContact (secondBody, secondShape, firstBody, firstShape);
            }
        }
    }
}

void SimulationExecutor::PostSolve (b2Contact *_contact, [[maybe_unused]] const b2ContactImpulse *_impulse) noexcept
{
    const b2Fixture *firstShape = _contact->GetFixtureA ();
    const b2Fixture *secondShape = _contact->GetFixtureB ();
    EMERGENCE_ASSERT (!firstShape->IsSensor () && !secondShape->IsSensor ());

    const bool updateCollisionContactOnFirst = IsMaintainingContactList (firstShape->GetFilterData ());
    const bool updateCollisionContactOnSecond = IsMaintainingContactList (secondShape->GetFilterData ());

    if (updateCollisionContactOnFirst || updateCollisionContactOnSecond)
    {
        b2WorldManifold contactWorldManifold;
        _contact->GetWorldManifold (&contactWorldManifold);

        auto updateContact =
            [this, _contact, &contactWorldManifold] (const b2Fixture *_shape, const b2Fixture *_otherShape)
        {
            struct
            {
                UniqueId shapeId;
                UniqueId otherShapeId;
            } query {_shape->GetUserData ().shapeId, _otherShape->GetUserData ().shapeId};

            auto cursor = modifyCollisionContactByShapeIdAndOtherShapeId.Execute (&query);
            auto *contact = static_cast<CollisionContact2d *> (*cursor);
            EMERGENCE_ASSERT (contact);

            contact->normal = FromBox2d (contactWorldManifold.normal);
            contact->points.Clear ();

            for (size_t pointIndex = 0u; pointIndex < static_cast<size_t> (_contact->GetManifold ()->pointCount);
                 ++pointIndex)
            {
                [[maybe_unused]] const bool emplaceSuccessful =
                    contact->points.TryEmplaceBack (FromBox2d (contactWorldManifold.points[pointIndex]));
                EMERGENCE_ASSERT (emplaceSuccessful);
            }
        };

        if (updateCollisionContactOnFirst)
        {
            updateContact (firstShape, secondShape);
        }

        if (updateCollisionContactOnSecond)
        {
            updateContact (secondShape, firstShape);
        }
    }
}

void SimulationExecutor::EndContact (b2Contact *_contact) noexcept
{
    const b2Fixture *firstShape = _contact->GetFixtureA ();
    const b2Fixture *secondShape = _contact->GetFixtureB ();

    const bool firstSensor = firstShape->IsSensor ();
    const bool secondSensor = secondShape->IsSensor ();
    EMERGENCE_ASSERT (!firstSensor || !secondSensor);

    if (firstSensor)
    {
        struct
        {
            UniqueId triggerShapeId;
            UniqueId intruderShapeId;
        } query {firstShape->GetUserData ().shapeId, secondShape->GetUserData ().shapeId};

        for (auto cursor = modifyTriggerContactByTriggerShapeIdAndIntruderShapeId.Execute (&query); *cursor;)
        {
            ~cursor;
        }
    }
    else if (secondSensor)
    {
        struct
        {
            UniqueId triggerShapeId;
            UniqueId intruderShapeId;
        } query {secondShape->GetUserData ().shapeId, firstShape->GetUserData ().shapeId};

        for (auto cursor = modifyTriggerContactByTriggerShapeIdAndIntruderShapeId.Execute (&query); *cursor;)
        {
            ~cursor;
        }
    }
    else
    {
        if (IsMaintainingContactList (firstShape->GetFilterData ()))
        {
            struct
            {
                UniqueId shapeId;
                UniqueId otherShapeId;
            } query {firstShape->GetUserData ().shapeId, secondShape->GetUserData ().shapeId};

            for (auto cursor = modifyCollisionContactByShapeIdAndOtherShapeId.Execute (&query); *cursor;)
            {
                ~cursor;
            }
        }

        if (IsMaintainingContactList (secondShape->GetFilterData ()))
        {
            struct
            {
                UniqueId shapeId;
                UniqueId otherShapeId;
            } query {secondShape->GetUserData ().shapeId, firstShape->GetUserData ().shapeId};

            for (auto cursor = modifyCollisionContactByShapeIdAndOtherShapeId.Execute (&query); *cursor;)
            {
                ~cursor;
            }
        }
    }
}

void SimulationExecutor::SyncBodiesWithOutsideManipulations () noexcept
{
    for (auto bodyCursor = editBodyWithOutsideManipulations.Execute ();
         auto *body = static_cast<RigidBody2dComponent *> (*bodyCursor); ++bodyCursor)
    {
        auto *box2dBody = static_cast<b2Body *> (body->implementationHandle);
        box2dBody->SetGravityScale (body->affectedByGravity ? 1.0f : 0.0f);

        auto transformCursor = fetchTransformByObjectId.Execute (&body->objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            // Transformless bodies must've been removed by other routines.
            EMERGENCE_ASSERT (false);
            continue;
        }

        const Math::Transform2d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        box2dBody->SetTransform (ToBox2d (logicalTransform.translation), logicalTransform.rotation);

        if (body->type != RigidBody2dType::STATIC)
        {
            box2dBody->SetBullet (body->continuousCollisionDetection);
            box2dBody->SetLinearVelocity (ToBox2d (body->linearVelocity));
            box2dBody->SetAngularVelocity (body->angularVelocity);
        }

        if (body->type == RigidBody2dType::DYNAMIC)
        {
            box2dBody->SetLinearDamping (body->linearDamping);
            box2dBody->SetAngularDamping (body->angularDamping);
            box2dBody->SetFixedRotation (body->fixedRotation);

            if (!Math::NearlyEqual (body->additiveLinearImpulse, Math::Vector2f::ZERO))
            {
                box2dBody->ApplyLinearImpulseToCenter (ToBox2d (body->additiveLinearImpulse), true);
            }

            if (!Math::NearlyEqual (body->additiveAngularImpulse, 0.0f))
            {
                box2dBody->ApplyAngularImpulse (body->additiveAngularImpulse, true);
            }
        }

        // TODO: Technically, we need this only because scale might have been changed. Any way to optimise that?
        for (auto shapeCursor = fetchShapeByObjectId.Execute (&body->objectId);
             const auto *shape = static_cast<const CollisionShape2dComponent *> (*shapeCursor); ++shapeCursor)
        {
            UpdateShapeGeometryAndPose (shape, logicalTransform.scale);
        }
    }
}

void SimulationExecutor::ExecuteSimulation (const PhysicsWorld2dSingleton *_physicsWorld, float _timeStep) noexcept
{
    if (!Math::NearlyEqual (_timeStep, 0.0f))
    {
        auto *box2dWorld = block_cast<b2World *> (_physicsWorld->implementationBlock);
        box2dWorld->SetContactFilter (this);
        box2dWorld->SetContactListener (this);

        // TODO: Make iteration constants part of public API?
        box2dWorld->Step (_timeStep, 8u, 3u);

        box2dWorld->SetContactFilter (nullptr);
        box2dWorld->SetContactListener (nullptr);
    }
}

void SimulationExecutor::SyncKinematicAndDynamicBodies () noexcept
{
    auto syncTransform = [this] (RigidBody2dComponent *_body)
    {
        auto *box2dBody = static_cast<b2Body *> (_body->implementationHandle);
        auto transformCursor = editTransformByObjectId.Execute (&_body->objectId);

        if (auto *transform = static_cast<Transform2dComponent *> (*transformCursor))
        {
            const Math::Vector2f &scale = transform->GetLogicalLocalTransform ().scale;
            // Currently, we assume that non-static bodies are attached to transform root elements only.
            EMERGENCE_ASSERT (transform->GetParentObjectId () == INVALID_UNIQUE_ID);
            transform->SetLogicalLocalTransform (
                {FromBox2d (box2dBody->GetPosition ()), box2dBody->GetAngle (), scale});
        }
    };

    for (auto kinematicCursor = editKinematicBody.Execute ();
         auto *body = static_cast<RigidBody2dComponent *> (*kinematicCursor); ++kinematicCursor)
    {
        syncTransform (body);
    }

    for (auto dynamicCursor = editDynamicBody.Execute ();
         auto *body = static_cast<RigidBody2dComponent *> (*dynamicCursor); ++dynamicCursor)
    {
        auto *box2dBody = static_cast<b2Body *> (body->implementationHandle);
        if (!body->ignoreSimulationVelocityChange)
        {
            body->linearVelocity = FromBox2d (box2dBody->GetLinearVelocity ());
            body->angularVelocity = box2dBody->GetAngularVelocity ();
        }

        body->additiveLinearImpulse = Math::Vector2f::ZERO;
        body->additiveAngularImpulse = 0.0f;
        syncTransform (body);
    }
}

static Memory::Heap box2dHeap {Memory::Profiler::AllocationGroup {"Physics2d::Box2d"_us}};

void *Box2dProfiledAllocation (std::int32_t _size) noexcept
{
    auto *memory = static_cast<uintptr_t *> (box2dHeap.Acquire (_size + sizeof (uintptr_t), sizeof (uintptr_t)));
    *memory = _size;
    return memory + 1u;
}

void Box2dProfiledFree (void *_memory) noexcept
{
    uintptr_t *acquiredMemory = static_cast<uintptr_t *> (_memory) - 1u;
    box2dHeap.Release (acquiredMemory, *acquiredMemory + sizeof (uintptr_t));
}

void *Box2dEffectiveAllocation (std::int32_t _size) noexcept
{
    return malloc (_size);
}

void Box2dEffectiveFree (void *_memory) noexcept
{
    free (_memory);
}

void Box2dLog (const char *_information) noexcept
{
    EMERGENCE_LOG (INFO, "Physics2d::Box2d: ", _information);
}

b2Vec2 ToBox2d (const Math::Vector2f &_vector) noexcept
{
    return {_vector.x, _vector.y};
}

Math::Vector2f FromBox2d (const b2Vec2 &_vector) noexcept
{
    return {_vector.x, _vector.y};
}

static b2Shape::Type ToBox2d (CollisionGeometry2dType _type) noexcept
{
    switch (_type)
    {
    case CollisionGeometry2dType::BOX:
        return b2Shape::e_polygon;

    case CollisionGeometry2dType::CIRCLE:
        return b2Shape::e_circle;

    case CollisionGeometry2dType::LINE:
        return b2Shape::e_edge;
    }

    EMERGENCE_ASSERT (false);
    return b2Shape::e_typeCount;
}

b2Filter ConstructBox2dFilter (uint8_t _collisionGroup, bool _maintainContactList, bool _visibleToWorldQueries) noexcept
{
    b2Filter filter;
    filter.categoryBits = static_cast<uint16_t> (_collisionGroup);
    filter.maskBits = _maintainContactList ? 1u : 0u;
    filter.groupIndex = _visibleToWorldQueries ? 1u : 0u;
    return filter;
}

uint8_t GetCollisionGroup (const b2Filter &_filter) noexcept
{
    return static_cast<uint8_t> (_filter.categoryBits);
}

bool IsMaintainingContactList (const b2Filter &_filter) noexcept
{
    return _filter.maskBits == 1u;
}

// TODO: Right now world queries are not implemented, therefore this method is unused.
[[maybe_unused]] bool IsVisibleToWorldQueries (const b2Filter &_filter) noexcept
{
    return _filter.groupIndex == 1u;
}

void UpdateShapeMaterial (b2Fixture *_fixture, const DynamicsMaterial2d *_material) noexcept
{
    _fixture->SetFriction (_material->friction);
    _fixture->SetRestitution (_material->restitution);
    _fixture->SetRestitutionThreshold (_material->restitutionThreshold);
    _fixture->SetDensity (_material->density);
}

bool UpdateShapeGeometryAndPose (const CollisionShape2dComponent *_shape, const Math::Vector2f &_worldScale) noexcept
{
    auto *fixture = static_cast<b2Fixture *> (_shape->implementationHandle);
    if (!fixture)
    {
        EMERGENCE_ASSERT (!_shape->enabled);
        return true;
    }

    if (fixture->GetShape ()->GetType () != ToBox2d (_shape->geometry.type))
    {
        EMERGENCE_LOG (ERROR,
                       "Physics2d: Unable to update CollisionShape2dComponent geometry, because changing geometry type "
                       "is forbidden!");
        return false;
    }

    switch (_shape->geometry.type)
    {
    case CollisionGeometry2dType::BOX:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Type is known due to enum.
        UpdateBoxGeometryAndPose (_shape, _worldScale, static_cast<b2PolygonShape *> (fixture->GetShape ()));
        break;

    case CollisionGeometry2dType::CIRCLE:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Type is known due to enum.
        UpdateCircleGeometryAndPose (_shape, _worldScale, static_cast<b2CircleShape *> (fixture->GetShape ()));
        break;

    case CollisionGeometry2dType::LINE:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Type is known due to enum.
        UpdateLineGeometryAndPose (_shape, _worldScale, static_cast<b2EdgeShape *> (fixture->GetShape ()));
        break;
    }

    return true;
}

static void UpdateBoxGeometryAndPose (const CollisionShape2dComponent *_shape,
                                      const Math::Vector2f &_worldScale,
                                      b2PolygonShape *_polygonShape) noexcept
{
    _polygonShape->SetAsBox (_shape->geometry.boxHalfExtents.x * _worldScale.x,
                             _shape->geometry.boxHalfExtents.y * _worldScale.y,
                             ToBox2d (_shape->translation * _worldScale), _shape->rotation);
}

static void UpdateCircleGeometryAndPose (const CollisionShape2dComponent *_shape,
                                         const Math::Vector2f &_worldScale,
                                         b2CircleShape *_circleShape) noexcept
{
    _circleShape->m_radius = _shape->geometry.circleRadius * _worldScale.x;
    EMERGENCE_ASSERT (Math::NearlyEqual (_worldScale.x, _worldScale.y));
    _circleShape->m_p = ToBox2d (_shape->translation * _worldScale);
}

static void UpdateLineGeometryAndPose (const CollisionShape2dComponent *_shape,
                                       const Math::Vector2f &_worldScale,
                                       b2EdgeShape *_lineShape) noexcept
{
    const Math::Matrix3x3f localTransformMatrix =
        Math::Matrix3x3f {Math::Transform2d {Math::Vector2f::ZERO, 0.0f, _worldScale}} *
        Math::Matrix3x3f {Math::Transform2d {_shape->translation, _shape->rotation, Math::Vector2f::ONE}};

    const Math::Vector3f firstPoint3d =
        localTransformMatrix * Math::Vector3f {_shape->geometry.line.start.x, _shape->geometry.line.start.y, 1.0f};
    const Math::Vector2f firstPoint2d {firstPoint3d.x, firstPoint3d.y};

    const Math::Vector3f secondPoint3d =
        localTransformMatrix * Math::Vector3f {_shape->geometry.line.end.x, _shape->geometry.line.end.y, 1.0f};
    const Math::Vector2f secondPoint2d {secondPoint3d.x, secondPoint3d.y};

    _lineShape->SetTwoSided (ToBox2d (firstPoint2d), ToBox2d (secondPoint2d));
}

void ConstructBox2dShape (CollisionShape2dComponent *_shape,
                          b2Body *_body,
                          const DynamicsMaterial2d *_material,
                          const Math::Vector2f &_worldScale) noexcept
{
    static b2PolygonShape polygonShape;
    static b2CircleShape circleShape;
    static b2EdgeShape edgeShape;

    b2FixtureDef fixtureDefinition;
    switch (_shape->geometry.type)
    {
    case CollisionGeometry2dType::BOX:
        UpdateBoxGeometryAndPose (_shape, _worldScale, &polygonShape);
        fixtureDefinition.shape = &polygonShape;
        break;

    case CollisionGeometry2dType::CIRCLE:
        UpdateCircleGeometryAndPose (_shape, _worldScale, &circleShape);
        fixtureDefinition.shape = &circleShape;
        break;

    case CollisionGeometry2dType::LINE:
        UpdateLineGeometryAndPose (_shape, _worldScale, &edgeShape);
        fixtureDefinition.shape = &edgeShape;
        break;
    }

    fixtureDefinition.friction = _material->friction;
    fixtureDefinition.restitution = _material->restitution;
    fixtureDefinition.restitutionThreshold = _material->restitutionThreshold;
    fixtureDefinition.density = _material->density;

    fixtureDefinition.isSensor = _shape->trigger;
    fixtureDefinition.filter =
        ConstructBox2dFilter (_shape->collisionGroup, _shape->maintainCollisionContacts, _shape->visibleToWorldQueries);
    fixtureDefinition.userData.shapeId = _shape->shapeId;
    _shape->implementationHandle = _body->CreateFixture (&fixtureDefinition);
}

const Memory::UniqueString Checkpoint::STARTED {"Physics2dSimulationStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"Physics2dSimulationFinished"};

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    _pipelineBuilder.AddTask ("Physics2d::RemoveBodies"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, RigidBody2dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Physics2d::RemoveShapes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, CollisionShape2dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Physics2dSimulation");
    _pipelineBuilder.AddCheckpoint (Physics2dSimulation::Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Physics2dSimulation::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask (TaskNames::UPDATE_WORLD).SetExecutor<WorldUpdater> ();
    _pipelineBuilder.AddTask (TaskNames::SYNC_MATERIAL_CHANGES).SetExecutor<MaterialChangesSynchronizer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_MATERIAL_DELETION).SetExecutor<MaterialDeleter> ();

    _pipelineBuilder.AddTask (TaskNames::INITIALIZE_SHAPES).SetExecutor<ShapeInitializer> ();
    _pipelineBuilder.AddTask (TaskNames::SYNC_SHAPE_CHANGES).SetExecutor<ShapeChangesSynchronizer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_SHAPE_DELETION).SetExecutor<ShapeDeleter> ();

    _pipelineBuilder.AddTask (TaskNames::INITIALIZE_BODIES).SetExecutor<BodyInitializer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_BODY_DELETION).SetExecutor<BodyDeleter> ();

    _pipelineBuilder.AddTask (TaskNames::SYNC_BODY_MASSES).SetExecutor<BodyMassSynchronizer> ();
    _pipelineBuilder.AddTask (TaskNames::EXECUTE_SIMULATION).SetExecutor<SimulationExecutor> ();
}
} // namespace Emergence::Celerity::Physics2dSimulation
