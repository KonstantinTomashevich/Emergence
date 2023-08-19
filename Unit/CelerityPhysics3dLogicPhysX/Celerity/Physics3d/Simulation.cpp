#include <API/Common/BlockCast.hpp>
#include <API/Common/MuteWarnings.hpp>

#include <Celerity/TimeSingleton.hpp>
#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>
#include <Celerity/Physics3d/Events.hpp>
#include <Celerity/Physics3d/PhysXAccessSingleton.hpp>
#include <Celerity/Physics3d/PhysXInternalUtils.hpp>
#include <Celerity/Physics3d/PhysXWorld.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/Physics3d/RigidBody3dComponent.hpp>
#include <Celerity/Physics3d/Simulation.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

BEGIN_MUTING_WARNINGS
#include <PxMaterial.h>
#include <PxPhysicsVersion.h>
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <PxSimulationEventCallback.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxRigidBodyExt.h>
#include <pvd/PxPvdTransport.h>
END_MUTING_WARNINGS

namespace Emergence::Celerity::Physics3dSimulation
{
namespace TaskNames
{
// TODO: Currently all tasks are sequential and are separated only to make event pipeline work properly.
//       It seems that parallelization is not possible, because of close logical coupling between bodies,
//       shapes and materials. Is there any way to make it better?

static const Memory::UniqueString UPDATE_WORLD {"Physics3d::UpdateWorld"};

static const Memory::UniqueString INITIALIZE_MATERIALS {"Physics3d::InitializeMaterials"};
static const Memory::UniqueString SYNC_MATERIAL_CHANGES {"Physics3d::SyncMaterialChanges"};
static const Memory::UniqueString APPLY_MATERIAL_DELETION {"Physics3d::ApplyMaterialDeletion"};

static const Memory::UniqueString INITIALIZE_SHAPES {"Physics3d::InitializeShapes"};
static const Memory::UniqueString SYNC_SHAPE_CHANGES {"Physics3d::SyncShapeChanges"};
static const Memory::UniqueString APPLY_SHAPE_DELETION {"Physics3d::ApplyShapeDeletion"};

static const Memory::UniqueString INITIALIZE_BODIES {"Physics3d::InitializeBodies"};
static const Memory::UniqueString APPLY_BODY_DELETION {"Physics3d::ApplyBodyDeletion"};

static const Memory::UniqueString SYNC_BODY_MASSES {"Physics3d::SyncBodyMasses"};
static const Memory::UniqueString EXECUTE_SIMULATION {"Physics3d::ExecuteSimulation"};
} // namespace TaskNames

// NOTE: Every task must register physics world singleton modify access if it works directly with PhysX objects.
//       It allows us to be sure that there is no multithreaded PhysX calls.

static physx::PxShapeFlags CalculateShapeFlags (const CollisionShape3dComponent *_shape) noexcept;

static physx::PxGeometryType::Enum ToPxGeometryType (CollisionGeometry3dType _type) noexcept;

static bool UpdateShapeGeometry (const CollisionShape3dComponent *_shape, const Math::Vector3f &_worldScale) noexcept;

static void UpdateShapeLocalPose (const CollisionShape3dComponent *_shape, const Math::Vector3f &_worldScale) noexcept;

static void UpdateShapeFilter (const CollisionShape3dComponent *_shape) noexcept;

static void ConstructPxShape (CollisionShape3dComponent *_shape,
                              const PhysXWorld &_pxWorld,
                              const physx::PxMaterial &_pxMaterial,
                              const Math::Vector3f &_worldScale) noexcept;

static physx::PxFilterFlags PhysicsFilterShader (physx::PxFilterObjectAttributes _attributes0,
                                                 physx::PxFilterData _filterData0,
                                                 physx::PxFilterObjectAttributes _attributes1,
                                                 physx::PxFilterData _filterData1,
                                                 physx::PxPairFlags &_pairFlags,
                                                 const void *_constantBlock,
                                                 physx::PxU32 /*unused*/);

class WorldUpdater final : public TaskExecutorBase<WorldUpdater>
{
public:
    WorldUpdater (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static void EnsurePhysicsWorldReady (const PhysicsWorld3dSingleton *_physicsWorld) noexcept;

    void UpdateConfiguration (const PhysicsWorld3dSingleton *_physicsWorld) noexcept;

    static void UpdateCollisionMask (const PhysicsWorld3dSingleton *_physicsWorld) noexcept;

    static void UpdateRemoteDebugging (const PhysicsWorld3dSingleton *_physicsWorld) noexcept;

    ModifySingletonQuery modifyPhysX;
    FetchSingletonQuery fetchPhysicsWorld;
    FetchSequenceQuery fetchConfigurationChangedEvents;
};

WorldUpdater::WorldUpdater (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld3dSingleton)),
      fetchConfigurationChangedEvents (FETCH_SEQUENCE (PhysicsWorld3dConfigurationChanged))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
}

void WorldUpdater::Execute () noexcept
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld3dSingleton *> (*physicsWorldCursor);
    EnsurePhysicsWorldReady (physicsWorld);
    UpdateRemoteDebugging (physicsWorld);
    UpdateConfiguration (physicsWorld);
}

void WorldUpdater::EnsurePhysicsWorldReady (const PhysicsWorld3dSingleton *_physicsWorld) noexcept
{
    // We are free to const cast here, because we're modifying PhysX-related objects,
    // which is under PhysXAccessSingleton access control.
    auto &pxWorld = const_cast<PhysXWorld &> (block_cast<PhysXWorld> (_physicsWorld->implementationBlock));

    if (!pxWorld.foundation)
    {
        physx::PxAllocatorCallback *allocator;
        if (_physicsWorld->enableMemoryProfiling)
        {
            pxWorld.allocator = ProfiledAllocator {pxWorld.heap.GetAllocationGroup ()};
            allocator = &std::get<ProfiledAllocator> (pxWorld.allocator);
        }
        else
        {
            pxWorld.allocator = physx::PxDefaultAllocator {};
            allocator = &std::get<physx::PxDefaultAllocator> (pxWorld.allocator);
        }

        pxWorld.foundation = PxCreateFoundation (PX_PHYSICS_VERSION, *allocator, pxWorld.errorCallback);
        pxWorld.remoteDebugger = physx::PxCreatePvd (*pxWorld.foundation);

        physx::PxTolerancesScale tolerance;
        tolerance.length = _physicsWorld->toleranceLength;
        tolerance.speed = _physicsWorld->toleranceSpeed;

        pxWorld.physics =
            PxCreatePhysics (PX_PHYSICS_VERSION, *pxWorld.foundation, tolerance, false, pxWorld.remoteDebugger);

        physx::PxSceneDesc sceneDescriptor {pxWorld.physics->getTolerancesScale ()};
        sceneDescriptor.gravity = {_physicsWorld->gravity.x, _physicsWorld->gravity.y, _physicsWorld->gravity.z};
        sceneDescriptor.cpuDispatcher = &pxWorld.dispatcher;
        sceneDescriptor.filterShader = PhysicsFilterShader;

        pxWorld.scene = pxWorld.physics->createScene (sceneDescriptor);
        UpdateCollisionMask (_physicsWorld);
    }
}

void WorldUpdater::UpdateConfiguration (const PhysicsWorld3dSingleton *_physicsWorld) noexcept
{
    auto eventCursor = fetchConfigurationChangedEvents.Execute ();
    if (!*eventCursor)
    {
        return;
    }

    const auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    pxWorld.scene->setGravity ({_physicsWorld->gravity.x, _physicsWorld->gravity.y, _physicsWorld->gravity.z});
    UpdateCollisionMask (_physicsWorld);
}

void WorldUpdater::UpdateCollisionMask (const PhysicsWorld3dSingleton *_physicsWorld) noexcept
{
    const auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    pxWorld.scene->setFilterShaderData (_physicsWorld->collisionMasks.data (), sizeof (_physicsWorld->collisionMasks));
}

void WorldUpdater::UpdateRemoteDebugging (const PhysicsWorld3dSingleton *_physicsWorld) noexcept
{
    // We are free to const cast here, because we're modifying PhysX-related objects,
    // which is under PhysXAccessSingleton access control.
    auto &pxWorld = const_cast<PhysXWorld &> (block_cast<PhysXWorld> (_physicsWorld->implementationBlock));

    if (pxWorld.remoteDebuggerEnabled != _physicsWorld->enableRemoteDebugger)
    {
        if (_physicsWorld->enableRemoteDebugger)
        {
            physx::PxPvdTransport *oldTransport = pxWorld.remoteDebugger->getTransport ();
            if (pxWorld.remoteDebugger->isConnected ())
            {
                pxWorld.remoteDebugger->disconnect ();
            }

            physx::PxPvdTransport *newTransport = physx::PxDefaultPvdSocketTransportCreate (
                _physicsWorld->remoteDebuggerUrl.data (), static_cast<int> (_physicsWorld->remoteDebuggerPort), 10);

            pxWorld.remoteDebuggerEnabled =
                pxWorld.remoteDebugger->connect (*newTransport, physx::PxPvdInstrumentationFlag::eALL);

            if (oldTransport)
            {
                oldTransport->release ();
            }
        }
        else
        {
            pxWorld.remoteDebugger->disconnect ();
            pxWorld.remoteDebuggerEnabled = false;
        }
    }
}

class MaterialInitializer final : public TaskExecutorBase<MaterialInitializer>
{
public:
    MaterialInitializer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyPhysX;
    FetchSingletonQuery fetchPhysicsWorld;
    ModifyValueQuery modifyMaterialById;

    FetchSequenceQuery fetchMaterialAddedFixedEvents;
    FetchSequenceQuery fetchMaterialAddedCustomEvents;
};

MaterialInitializer::MaterialInitializer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld3dSingleton)),
      modifyMaterialById (MODIFY_VALUE_1F (DynamicsMaterial3d, id)),
      fetchMaterialAddedFixedEvents (FETCH_SEQUENCE (DynamicsMaterial3dAddedFixedEvent)),
      fetchMaterialAddedCustomEvents (FETCH_SEQUENCE (DynamicsMaterial3dAddedCustomToFixedEvent))
{
    _constructor.DependOn (TaskNames::UPDATE_WORLD);
}

void MaterialInitializer::Execute () noexcept
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld3dSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (Memory::UniqueString _id)
    {
        auto materialCursor = modifyMaterialById.Execute (&_id);
        if (auto *material = static_cast<DynamicsMaterial3d *> (*materialCursor))
        {
            EMERGENCE_ASSERT (!material->implementationHandle);

            physx::PxMaterial *pxMaterial = pxWorld.physics->createMaterial (
                material->staticFriction, material->dynamicFriction, material->restitution);
            pxMaterial->setFlag (physx::PxMaterialFlag::eDISABLE_FRICTION, !material->enableFriction);
            material->implementationHandle = pxMaterial;
        }
        else
        {
            // Material is already removed, event was outdated.
        }
    };

    for (auto eventCursor = fetchMaterialAddedFixedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial3dAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->id);
    }

    for (auto eventCursor = fetchMaterialAddedCustomEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial3dAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->id);
    }
}

class MaterialChangesSynchronizer final : public TaskExecutorBase<MaterialChangesSynchronizer>
{
public:
    MaterialChangesSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyPhysX;
    FetchValueQuery fetchMaterialById;
    FetchSequenceQuery fetchMaterialChangedEvents;
    FetchValueQuery fetchShapeByMaterialId;
    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

MaterialChangesSynchronizer::MaterialChangesSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial3d, id)),
      fetchMaterialChangedEvents (FETCH_SEQUENCE (DynamicsMaterial3dChangedEvent)),
      fetchShapeByMaterialId (FETCH_VALUE_1F (CollisionShape3dComponent, materialId)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_MATERIALS);
}

void MaterialChangesSynchronizer::Execute () noexcept
{
    for (auto eventCursor = fetchMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial3dChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto materialCursor = fetchMaterialById.Execute (&event->id);
        if (const auto *material = static_cast<const DynamicsMaterial3d *> (*materialCursor))
        {
            auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
            pxMaterial->setStaticFriction (material->staticFriction);
            pxMaterial->setDynamicFriction (material->dynamicFriction);
            pxMaterial->setRestitution (material->restitution);
            pxMaterial->setFlag (physx::PxMaterialFlag::eDISABLE_FRICTION, !material->enableFriction);

            for (auto shapeCursor = fetchShapeByMaterialId.Execute (&material->id);
                 const auto *shape = static_cast<const CollisionShape3dComponent *> (*shapeCursor); ++shapeCursor)
            {
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
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
    ModifySingletonQuery modifyPhysX;
    FetchSequenceQuery fetchMaterialRemovedEvents;
    RemoveValueQuery removeShapeByMaterialId;
};

MaterialDeleter::MaterialDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchMaterialRemovedEvents (FETCH_SEQUENCE (DynamicsMaterial3dRemovedEvent)),
      removeShapeByMaterialId (REMOVE_VALUE_1F (CollisionShape3dComponent, materialId))
{
    _constructor.DependOn (TaskNames::SYNC_MATERIAL_CHANGES);
}

void MaterialDeleter::Execute () noexcept
{
    for (auto eventCursor = fetchMaterialRemovedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterial3dRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        auto *pxMaterial = static_cast<physx::PxMaterial *> (event->implementationHandle);
        for (auto shapeCursor = removeShapeByMaterialId.Execute (&event->id); shapeCursor.ReadConst ();)
        {
            // Shapes can not exist without material, therefore we are cleaning them up.
            ~shapeCursor;
        }

        if (pxMaterial)
        {
            pxMaterial->release ();
        }
    }
}

class ShapeInitializer final : public TaskExecutorBase<ShapeInitializer>
{
public:
    ShapeInitializer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyPhysX;
    FetchSingletonQuery fetchPhysicsWorld;

    ModifyValueQuery modifyShapeByShapeId;
    FetchValueQuery fetchMaterialById;
    FetchValueQuery fetchBodyByObjectId;

    FetchValueQuery fetchTransformByObjectId;
    Transform3dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchShapeAddedFixedEvents;
    FetchSequenceQuery fetchShapeAddedCustomEvents;
    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeInitializer::ShapeInitializer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld3dSingleton)),

      modifyShapeByShapeId (MODIFY_VALUE_1F (CollisionShape3dComponent, shapeId)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial3d, id)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody3dComponent, objectId)),

      fetchTransformByObjectId (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeAddedFixedEvents (FETCH_SEQUENCE (CollisionShape3dComponentAddedFixedEvent)),
      fetchShapeAddedCustomEvents (FETCH_SEQUENCE (CollisionShape3dComponentAddedCustomToFixedEvent)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_MATERIAL_DELETION);
}

void ShapeInitializer::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld3dSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (UniqueId _shapeId)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&_shapeId);
        auto *shape = static_cast<CollisionShape3dComponent *> (*shapeCursor);

        if (!shape)
        {
            // Shape is already removed.
            return;
        }

        auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
        const auto *material = static_cast<const DynamicsMaterial3d *> (*materialCursor);

        if (!material)
        {
            EMERGENCE_LOG (ERROR, "Physics3d: Unable to find DynamicsMaterial3d with id ", shape->materialId,
                           "! Shape, that attempts to use this material, will be deleted.");

            ~shapeCursor;
            return;
        }

        const auto *pxMaterial = static_cast<const physx::PxMaterial *> (material->implementationHandle);
        auto transformCursor = fetchTransformByObjectId.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics3d: Unable to add CollisionShape3dComponent to object with id ",
                           shape->objectId, ", because it has no Transform3dComponent!");

            ~shapeCursor;
            return;
        }

        const Math::Vector3f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        ConstructPxShape (shape, pxWorld, *pxMaterial, worldScale);
        auto bodyCursor = fetchBodyByObjectId.Execute (&shape->objectId);

        if (const auto *body = static_cast<const RigidBody3dComponent *> (*bodyCursor))
        {
            if (auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle))
            {
                pxBody->attachShape (*static_cast<physx::PxShape *> (shape->implementationHandle));
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
            }
            else
            {
                // Body is not initialized yet. Shape will be added during initialization.
            }
        }
    };

    for (auto eventCursor = fetchShapeAddedFixedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentAddedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->shapeId);
    }

    for (auto eventCursor = fetchShapeAddedCustomEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->shapeId);
    }
}

class ShapeChangesSynchronizer final : public TaskExecutorBase<ShapeChangesSynchronizer>
{
public:
    ShapeChangesSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void ApplyShapeMaterialChanges () noexcept;

    void ApplyShapeGeometryChanges () noexcept;

    void ApplyShapeAttributesChanges () noexcept;

    ModifySingletonQuery modifyPhysX;

    FetchValueQuery fetchShapeByShapeId;
    RemoveValueQuery removeShapeByShapeId;
    FetchValueQuery fetchMaterialById;

    FetchValueQuery fetchTransformById;
    Transform3dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchShapeMaterialChangedEvents;
    FetchSequenceQuery fetchShapeGeometryChangedEvents;
    FetchSequenceQuery fetchShapeAttributesChangedEvents;

    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeChangesSynchronizer::ShapeChangesSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchShapeByShapeId (FETCH_VALUE_1F (CollisionShape3dComponent, shapeId)),
      removeShapeByShapeId (REMOVE_VALUE_1F (CollisionShape3dComponent, shapeId)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial3d, id)),

      fetchTransformById (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeMaterialChangedEvents (FETCH_SEQUENCE (CollisionShape3dComponentMaterialChangedEvent)),
      fetchShapeGeometryChangedEvents (FETCH_SEQUENCE (CollisionShape3dComponentGeometryChangedEvent)),
      fetchShapeAttributesChangedEvents (FETCH_SEQUENCE (CollisionShape3dComponentAttributesChangedEvent)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_SHAPES);
}

void ShapeChangesSynchronizer::Execute ()
{
    ApplyShapeMaterialChanges ();
    ApplyShapeGeometryChanges ();
    ApplyShapeAttributesChanges ();
}

void ShapeChangesSynchronizer::ApplyShapeMaterialChanges () noexcept
{
    for (auto eventCursor = fetchShapeMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentMaterialChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShape3dComponent *> (shapeCursor.ReadConst ()))
        {
            auto cursor = insertBodyMassInvalidatedEvents.Execute ();
            static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
            auto materialCursor = fetchMaterialById.Execute (&shape->materialId);

            if (const auto *material = static_cast<const DynamicsMaterial3d *> (*materialCursor))
            {
                auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
                static_cast<physx::PxShape *> (shape->implementationHandle)->setMaterials (&pxMaterial, 1u);
            }
            else
            {
                EMERGENCE_LOG (ERROR, "Physics3d: Unable to find DynamicsMaterial3d with id ", shape->materialId,
                               "! Shape, that attempts to use this material, will be deleted.");

                ~shapeCursor;
                continue;
            }
        }
    }
}

void ShapeChangesSynchronizer::ApplyShapeGeometryChanges () noexcept
{
    for (auto eventCursor = fetchShapeGeometryChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentGeometryChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        const auto *shape = static_cast<const CollisionShape3dComponent *> (shapeCursor.ReadConst ());

        if (!shape)
        {
            continue;
        }

        auto cursor = insertBodyMassInvalidatedEvents.Execute ();
        static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
        auto transformCursor = fetchTransformById.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics3d: Unable to update CollisionShape3dComponent to object with id ",
                           shape->objectId, ", because it has no Transform3dComponent!");

            ~shapeCursor;
            continue;
        }

        const Math::Vector3f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        if (!UpdateShapeGeometry (shape, worldScale))
        {
            // Shape type was changed, therefore shape can not be correctly represented anymore.
            ~shapeCursor;
            continue;
        }

        UpdateShapeLocalPose (shape, worldScale);
    }
}

void ShapeChangesSynchronizer::ApplyShapeAttributesChanges () noexcept
{
    for (auto eventCursor = fetchShapeAttributesChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentAttributesChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = fetchShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShape3dComponent *> (*shapeCursor))
        {
            static_cast<physx::PxShape *> (shape->implementationHandle)->setFlags (CalculateShapeFlags (shape));
            UpdateShapeFilter (shape);

            auto cursor = insertBodyMassInvalidatedEvents.Execute ();
            static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
        }
    }
}

class ShapeDeleter final : public TaskExecutorBase<ShapeDeleter>
{
public:
    ShapeDeleter (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyPhysX;
    FetchValueQuery fetchBodyByObjectId;
    FetchSequenceQuery fetchShapeRemovedEvents;
    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeDeleter::ShapeDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody3dComponent, objectId)),
      fetchShapeRemovedEvents (FETCH_SEQUENCE (CollisionShape3dComponentRemovedEvent)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::SYNC_SHAPE_CHANGES);
}

void ShapeDeleter::Execute ()
{
    for (auto eventCursor = fetchShapeRemovedEvents.Execute ();
         const auto *event = static_cast<const CollisionShape3dComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto *pxShape = static_cast<physx::PxShape *> (event->implementationHandle))
        {
            auto bodyCursor = fetchBodyByObjectId.Execute (&event->objectId);
            if (const auto *body = static_cast<const RigidBody3dComponent *> (*bodyCursor))
            {
                static_cast<physx::PxRigidBody *> (body->implementationHandle)->detachShape (*pxShape);
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = body->objectId;
            }
            else
            {
                // Body was removed too.
            }

            pxShape->release ();
        }
    }
}

class BodyInitializer final : public TaskExecutorBase<BodyInitializer>
{
public:
    BodyInitializer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyPhysX;
    FetchSingletonQuery fetchPhysicsWorld;

    ModifyValueQuery modifyBodyByObjectId;
    FetchValueQuery fetchShapeByObjectId;
    FetchValueQuery fetchTransformByObjectId;
    Transform3dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchBodyAddedFixedEvents;
    FetchSequenceQuery fetchBodyAddedCustomEvents;
    InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

BodyInitializer::BodyInitializer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld3dSingleton)),

      modifyBodyByObjectId (MODIFY_VALUE_1F (RigidBody3dComponent, objectId)),
      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape3dComponent, objectId)),
      fetchTransformByObjectId (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchBodyAddedFixedEvents (FETCH_SEQUENCE (RigidBody3dComponentAddedFixedEvent)),
      fetchBodyAddedCustomEvents (FETCH_SEQUENCE (RigidBody3dComponentAddedCustomToFixedEvent)),
      insertBodyMassInvalidatedEvents (INSERT_SHORT_TERM (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_SHAPE_DELETION);
}

void BodyInitializer::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld3dSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (UniqueId _objectId)
    {
        auto bodyCursor = modifyBodyByObjectId.Execute (&_objectId);
        auto *body = static_cast<RigidBody3dComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed.
            return;
        }

        auto transformCursor = fetchTransformByObjectId.Execute (&_objectId);
        const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics3d: Unable to initialize RigidBody3dComponent on object with id ",
                           body->objectId, ", because it has no Transform3dComponent!");

            ~bodyCursor;
            return;
        }

        const Math::Transform3d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        const physx::PxTransform pxTransform {ToPhysX (logicalTransform.translation),
                                              ToPhysX (logicalTransform.rotation)};

        physx::PxRigidActor *pxActor = nullptr;
        switch (body->type)
        {
        case RigidBody3dType::STATIC:
        {
            pxActor = pxWorld.physics->createRigidStatic (pxTransform);
            break;
        }

        case RigidBody3dType::KINEMATIC:
        {
            physx::PxRigidDynamic *pxBody = pxWorld.physics->createRigidDynamic (pxTransform);
            pxActor = pxBody;
            pxBody->setRigidBodyFlag (physx::PxRigidBodyFlag::eKINEMATIC, true);
            break;
        }

        case RigidBody3dType::DYNAMIC:
        {
            physx::PxRigidDynamic *pxBody = pxWorld.physics->createRigidDynamic (pxTransform);
            pxActor = pxBody;

            pxBody->setLinearDamping (body->linearDamping);
            pxBody->setAngularDamping (body->angularDamping);
            pxBody->setRigidBodyFlag (physx::PxRigidBodyFlag::eENABLE_CCD, body->continuousCollisionDetection);

            pxBody->setLinearVelocity (ToPhysX (body->linearVelocity));
            pxBody->setAngularVelocity (ToPhysX ({body->angularVelocity}));
            pxBody->setRigidDynamicLockFlags (physx::PxRigidDynamicLockFlags {body->lockFlags});
            break;
        }
        }

        body->implementationHandle = pxActor;
        pxActor->setActorFlag (physx::PxActorFlag::eDISABLE_GRAVITY, !body->affectedByGravity);
        pxActor->userData = body;

        for (auto shapeCursor = fetchShapeByObjectId.Execute (&body->objectId);
             const auto *shape = static_cast<const CollisionShape3dComponent *> (*shapeCursor); ++shapeCursor)
        {
            pxActor->attachShape (*static_cast<physx::PxShape *> (shape->implementationHandle));
        }

        pxWorld.scene->addActor (*pxActor);
        if (body->type == RigidBody3dType::DYNAMIC)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Type is known due to enum.
            auto *pxBody = static_cast<physx::PxRigidDynamic *> (pxActor);

            if (!Math::NearlyEqual (body->additiveLinearImpulse, Math::Vector3f::ZERO))
            {
                pxBody->addForce (ToPhysX (body->additiveLinearImpulse), physx::PxForceMode::eIMPULSE);
            }

            if (!Math::NearlyEqual (body->additiveAngularImpulse, Math::Vector3f::ZERO))
            {
                pxBody->addTorque (ToPhysX (body->additiveAngularImpulse), physx::PxForceMode::eIMPULSE);
            }
        }

        // We can calculate body mass more effectively here, but there is no
        // sense to add complexity, because body addition is not common event.
        auto cursor = insertBodyMassInvalidatedEvents.Execute ();
        static_cast<RigidBody3dComponentMassInvalidatedEvent *> (++cursor)->objectId = body->objectId;
    };

    for (auto eventCursor = fetchBodyAddedFixedEvents.Execute ();
         const auto *event = static_cast<const RigidBody3dComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->objectId);
    }

    for (auto eventCursor = fetchBodyAddedCustomEvents.Execute ();
         const auto *event = static_cast<const RigidBody3dComponentAddedCustomToFixedEvent *> (*eventCursor);
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
    ModifySingletonQuery modifyPhysX;
    FetchSequenceQuery fetchBodyRemovedEvents;
};

BodyDeleter::BodyDeleter (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchBodyRemovedEvents (FETCH_SEQUENCE (RigidBody3dComponentRemovedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_BODIES);
}

void BodyDeleter::Execute ()
{
    for (auto eventCursor = fetchBodyRemovedEvents.Execute ();
         const auto *event = static_cast<const RigidBody3dComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto *pxActor = static_cast<physx::PxRigidActor *> (event->implementationHandle))
        {
            pxActor->release ();
        }
    }
}

class BodyMassSynchronizer final : public TaskExecutorBase<BodyMassSynchronizer>
{
public:
    BodyMassSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    ModifySingletonQuery modifyPhysX;
    FetchValueQuery fetchBodyByObjectId;
    FetchValueQuery fetchShapeByObjectId;
    FetchValueQuery fetchMaterialById;
    FetchSequenceQuery fetchBodyMassInvalidationEvents;
};

BodyMassSynchronizer::BodyMassSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchBodyByObjectId (FETCH_VALUE_1F (RigidBody3dComponent, objectId)),
      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape3dComponent, objectId)),
      fetchMaterialById (FETCH_VALUE_1F (DynamicsMaterial3d, id)),
      fetchBodyMassInvalidationEvents (FETCH_SEQUENCE (RigidBody3dComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_BODY_DELETION);
}

void BodyMassSynchronizer::Execute ()
{
    // Body mass recalculation is a rare task, therefore we can afford to allocate memory dynamically.
    Container::Vector<physx::PxShape *> shapes {heap.GetAllocationGroup ()};
    Container::Vector<float> densities {heap.GetAllocationGroup ()};
    Container::Vector<UniqueId> bodyIds {heap.GetAllocationGroup ()};

    // Filter out duplicates to avoid excessive mass recalculations (they're quite expensive).
    for (auto eventCursor = fetchBodyMassInvalidationEvents.Execute ();
         const auto *event = static_cast<const RigidBody3dComponentMassInvalidatedEvent *> (*eventCursor);
         ++eventCursor)
    {
        Container::AddUnique (bodyIds, event->objectId);
    }

    for (UniqueId objectId : bodyIds)
    {
        auto bodyCursor = fetchBodyByObjectId.Execute (&objectId);
        const auto *body = static_cast<const RigidBody3dComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed, no need to bother with recalculation.
            continue;
        }

        if (body->type != RigidBody3dType::DYNAMIC)
        {
            // Static bodies have no mass.
            continue;
        }

        auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle);
        shapes.resize (pxBody->getNbShapes ());
        pxBody->getShapes (shapes.data (), static_cast<physx::PxU32> (shapes.size ()));

        for (auto iterator = shapes.begin (); iterator != shapes.end ();)
        {
            if ((*iterator)->getFlags () & physx::PxShapeFlag::eSIMULATION_SHAPE)
            {
                ++iterator;
            }
            else
            {
                iterator = shapes.erase (iterator);
            }
        }

        if (shapes.empty ())
        {
            continue;
        }

        densities.resize (shapes.size ());
        std::uint32_t shapesFound = 0u;

        for (auto shapeCursor = fetchShapeByObjectId.Execute (&objectId);
             const auto *shape = static_cast<const CollisionShape3dComponent *> (*shapeCursor); ++shapeCursor)
        {
            auto iterator =
                std::find (shapes.begin (), shapes.end (), static_cast<physx::PxShape *> (shape->implementationHandle));

            if (iterator == shapes.end ())
            {
                // Not all shapes are attached: this case will be processed inside other routine.
                break;
            }

            auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
            const auto *material = static_cast<const DynamicsMaterial3d *> (*materialCursor);

            if (!material)
            {
                // Shapes can not exist without material. How this happened?
                EMERGENCE_ASSERT (false);
                break;
            }

            densities[iterator - shapes.begin ()] = material->density;
            ++shapesFound;
        }

        if (shapesFound == shapes.size ())
        {
            physx::PxRigidBodyExt::updateMassAndInertia (*pxBody, densities.data (),
                                                         static_cast<physx::PxU32> (densities.size ()));
        }
        else
        {
            // Body is not ready for recalculation. It will be called from other routine when body will be ready.
        }
    }
}

class SimulationExecutor final : public TaskExecutorBase<SimulationExecutor>, public physx::PxSimulationEventCallback
{
public:
    SimulationExecutor (TaskConstructor &_constructor) noexcept;

    void Execute ();

    void onConstraintBreak (physx::PxConstraintInfo * /*unused*/, physx::PxU32 /*unused*/) override;

    void onWake (physx::PxActor ** /*unused*/, physx::PxU32 /*unused*/) override;

    void onSleep (physx::PxActor ** /*unused*/, physx::PxU32 /*unused*/) override;

    void onContact (const physx::PxContactPairHeader &_pairHeader,
                    const physx::PxContactPair *_pairs,
                    physx::PxU32 _nbPairs) override;

    void onTrigger (physx::PxTriggerPair *_pairs, physx::PxU32 _count) override;

    void onAdvance (const physx::PxRigidBody *const * /*unused*/,
                    const physx::PxTransform * /*unused*/,
                    physx::PxU32 /*unused*/) override;

private:
    void SyncBodiesWithOutsideManipulations () noexcept;

    void UpdateKinematicTargets (float _timeStep) noexcept;

    void ExecuteSimulation (const PhysicsWorld3dSingleton *_physicsWorld, float _timeStep) noexcept;

    void SyncKinematicAndDynamicBodies () noexcept;

    ModifySingletonQuery modifyPhysX;
    FetchSingletonQuery fetchPhysicsWorld;
    FetchSingletonQuery fetchTime;

    FetchValueQuery fetchShapeByObjectId;
    EditSignalQuery editBodyWithOutsideManipulations;
    EditSignalQuery editKinematicBody;
    EditSignalQuery editDynamicBody;

    FetchValueQuery fetchTransformByObjectId;
    EditValueQuery editTransformByObjectId;
    Transform3dWorldAccessor transformWorldAccessor;

    InsertShortTermQuery insertContactFoundEvents;
    InsertShortTermQuery insertContactPersistsEvents;
    InsertShortTermQuery insertContactLostEvents;

    InsertShortTermQuery insertTriggerEnteredEvents;
    InsertShortTermQuery insertTriggerExitedEvents;
};

SimulationExecutor::SimulationExecutor (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysX (MODIFY_SINGLETON (PhysXAccessSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (PhysicsWorld3dSingleton)),
      fetchTime (FETCH_SINGLETON (TimeSingleton)),

      fetchShapeByObjectId (FETCH_VALUE_1F (CollisionShape3dComponent, objectId)),
      editBodyWithOutsideManipulations (EDIT_SIGNAL (RigidBody3dComponent, manipulatedOutsideOfSimulation, true)),
      editKinematicBody (EDIT_SIGNAL (RigidBody3dComponent, type, RigidBody3dType::KINEMATIC)),
      editDynamicBody (EDIT_SIGNAL (RigidBody3dComponent, type, RigidBody3dType::DYNAMIC)),

      fetchTransformByObjectId (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      editTransformByObjectId (EDIT_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertContactFoundEvents (INSERT_SHORT_TERM (Contact3dFoundEvent)),
      insertContactPersistsEvents (INSERT_SHORT_TERM (Contact3dPersistsEvent)),
      insertContactLostEvents (INSERT_SHORT_TERM (Contact3dLostEvent)),

      insertTriggerEnteredEvents (INSERT_SHORT_TERM (Trigger3dEnteredEvent)),
      insertTriggerExitedEvents (INSERT_SHORT_TERM (Trigger3dExitedEvent))
{
    _constructor.DependOn (TaskNames::SYNC_BODY_MASSES);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void SimulationExecutor::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorld3dSingleton *> (*physicsWorldCursor);

    if (!physicsWorld->simulating)
    {
        return;
    }

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    SyncBodiesWithOutsideManipulations ();
    UpdateKinematicTargets (time->fixedDurationS);
    ExecuteSimulation (physicsWorld, time->fixedDurationS);
    SyncKinematicAndDynamicBodies ();
}

void SimulationExecutor::onConstraintBreak (physx::PxConstraintInfo * /*unused*/, physx::PxU32 /*unused*/)
{
    // No events right now.
}

void SimulationExecutor::onWake (physx::PxActor ** /*unused*/, physx::PxU32 /*unused*/)
{
    // No events right now.
}

void SimulationExecutor::onSleep (physx::PxActor ** /*unused*/, physx::PxU32 /*unused*/)
{
    // No events right now.
}

void SimulationExecutor::onContact (const physx::PxContactPairHeader &_pairHeader,
                                    const physx::PxContactPair *_pairs,
                                    physx::PxU32 _nbPairs)
{
    if (_pairHeader.flags &
        (physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1))
    {
        // There is no way to send correct event after shapes or bodies were removed.
        // User logic should listen to removal events to be aware of such situations.
        return;
    }

    for (std::uint32_t index = 0u; index < _nbPairs; ++index)
    {
        const physx::PxContactPair &pair = _pairs[index];
        if (pair.flags & (physx::PxContactPairFlag::eREMOVED_SHAPE_0 | physx::PxContactPairFlag::eREMOVED_SHAPE_1))
        {
            // Same as for bodies.
            continue;
        }

        const auto *firstShape = static_cast<const CollisionShape3dComponent *> (pair.shapes[0u]->userData);
        const auto *secondShape = static_cast<const CollisionShape3dComponent *> (pair.shapes[1u]->userData);
        EMERGENCE_ASSERT (firstShape->sendContactEvents || secondShape->sendContactEvents);

#define FILL_EVENT                                                                                                     \
    event->firstObjectId = firstShape->objectId;                                                                       \
    event->firstShapeId = firstShape->shapeId;                                                                         \
    event->secondObjectId = secondShape->objectId;                                                                     \
    event->secondShapeId = secondShape->shapeId;

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            auto cursor = insertContactFoundEvents.Execute ();
            auto *event = static_cast<Contact3dFoundEvent *> (++cursor);
            FILL_EVENT
            event->initialContact = pair.flags & physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH;
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            auto cursor = insertContactPersistsEvents.Execute ();
            auto *event = static_cast<Contact3dPersistsEvent *> (++cursor);
            FILL_EVENT
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            auto cursor = insertContactLostEvents.Execute ();
            auto *event = static_cast<Contact3dLostEvent *> (++cursor);
            FILL_EVENT
            event->lastContact = pair.flags & physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH;
        }

#undef FILL_EVENT
    }
}

void SimulationExecutor::onTrigger (physx::PxTriggerPair *_pairs, physx::PxU32 _count)
{
    for (std::uint32_t index = 0u; index < _count; ++index)
    {
        const physx::PxTriggerPair &pair = _pairs[index];
        if (pair.flags &
            (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
        {
            // There is no way to send correct event after shapes or bodies were removed.
            // User logic should listen to removal events to be aware of such situations.
            continue;
        }

        const auto *triggerShape = static_cast<const CollisionShape3dComponent *> (pair.triggerShape->userData);
        const auto *intruderShape = static_cast<const CollisionShape3dComponent *> (pair.otherShape->userData);

#define FILL_EVENT                                                                                                     \
    event->triggerObjectId = triggerShape->objectId;                                                                   \
    event->triggerShapeId = triggerShape->shapeId;                                                                     \
    event->intruderObjectId = intruderShape->objectId;                                                                 \
    event->intruderShapeId = intruderShape->shapeId;

        if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            auto cursor = insertTriggerEnteredEvents.Execute ();
            auto *event = static_cast<Trigger3dEnteredEvent *> (++cursor);
            FILL_EVENT
        }
        else
        {
            EMERGENCE_ASSERT (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
            auto cursor = insertTriggerExitedEvents.Execute ();
            auto *event = static_cast<Trigger3dExitedEvent *> (++cursor);
            FILL_EVENT
        }

#undef FILL_EVENT
    }
}

void SimulationExecutor::onAdvance (const physx::PxRigidBody *const * /*unused*/,
                                    const physx::PxTransform * /*unused*/,
                                    const physx::PxU32 /*unused*/)
{
    // No events right now.
}

void SimulationExecutor::SyncBodiesWithOutsideManipulations () noexcept
{
    for (auto bodyCursor = editBodyWithOutsideManipulations.Execute ();
         auto *body = static_cast<RigidBody3dComponent *> (*bodyCursor); ++bodyCursor)
    {
        auto *pxActor = static_cast<physx::PxRigidActor *> (body->implementationHandle);
        pxActor->setActorFlag (physx::PxActorFlag::eDISABLE_GRAVITY, !body->affectedByGravity);

        auto transformCursor = fetchTransformByObjectId.Execute (&body->objectId);
        const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            // Transformless body must've been removed by other routines.
            EMERGENCE_ASSERT (false);
            continue;
        }

        const Math::Transform3d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        pxActor->setGlobalPose ({ToPhysX (logicalTransform.translation), ToPhysX (logicalTransform.rotation)});

        if (body->type == RigidBody3dType::DYNAMIC)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Type is known due to enum.
            auto *pxBody = static_cast<physx::PxRigidDynamic *> (pxActor);
            pxBody->setLinearDamping (body->linearDamping);
            pxBody->setAngularDamping (body->angularDamping);
            pxBody->setRigidBodyFlag (physx::PxRigidBodyFlag::eENABLE_CCD, body->continuousCollisionDetection);

            pxBody->setLinearVelocity (ToPhysX (body->linearVelocity));
            pxBody->setAngularVelocity (ToPhysX (body->angularVelocity));
            pxBody->setRigidDynamicLockFlags (physx::PxRigidDynamicLockFlags {body->lockFlags});

            if (!Math::NearlyEqual (body->additiveLinearImpulse, Math::Vector3f::ZERO))
            {
                pxBody->addForce (ToPhysX (body->additiveLinearImpulse), physx::PxForceMode::eIMPULSE);
            }

            if (!Math::NearlyEqual (body->additiveAngularImpulse, Math::Vector3f::ZERO))
            {
                pxBody->addTorque (ToPhysX (body->additiveAngularImpulse), physx::PxForceMode::eIMPULSE);
            }
        }

        for (auto shapeCursor = fetchShapeByObjectId.Execute (&body->objectId);
             const auto *shape = static_cast<const CollisionShape3dComponent *> (*shapeCursor); ++shapeCursor)
        {
            UpdateShapeGeometry (shape, logicalTransform.scale);
            UpdateShapeLocalPose (shape, logicalTransform.scale);
        }
    }
}

void SimulationExecutor::UpdateKinematicTargets (float _timeStep) noexcept
{
    for (auto kinematicCursor = editKinematicBody.Execute ();
         auto *body = static_cast<RigidBody3dComponent *> (*kinematicCursor); ++kinematicCursor)
    {
        auto *pxBody = static_cast<physx::PxRigidDynamic *> (body->implementationHandle);
        physx::PxTransform target = pxBody->getGlobalPose ();

        target.p += ToPhysX (body->linearVelocity * _timeStep);
        target.q *= ToPhysX (Math::Quaternion {body->angularVelocity * _timeStep});
        target.q.normalize ();

        pxBody->setKinematicTarget (target);
    }
}

void SimulationExecutor::ExecuteSimulation (const PhysicsWorld3dSingleton *_physicsWorld, float _timeStep) noexcept
{
    if (!Math::NearlyEqual (_timeStep, 0.0f))
    {
        const auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
        pxWorld.scene->setSimulationEventCallback (this);
        // TODO: Make use of scratch buffer?
        pxWorld.scene->simulate (_timeStep);
        pxWorld.scene->fetchResults (true);
    }
}

void SimulationExecutor::SyncKinematicAndDynamicBodies () noexcept
{
    auto syncTransform = [this] (RigidBody3dComponent *_body)
    {
        auto *pxBody = static_cast<physx::PxRigidBody *> (_body->implementationHandle);
        auto transformCursor = editTransformByObjectId.Execute (&_body->objectId);

        if (auto *transform = static_cast<Transform3dComponent *> (*transformCursor))
        {
            const physx::PxTransform &pxTransform = pxBody->getGlobalPose ();
            const Math::Vector3f &scale = transform->GetLogicalLocalTransform ().scale;

            // Currently, we assume that non-static bodies are attached to transform root elements only.
            EMERGENCE_ASSERT (transform->GetParentObjectId () == INVALID_UNIQUE_ID);
            transform->SetLogicalLocalTransform ({FromPhysX (pxTransform.p), FromPhysX (pxTransform.q), scale});
        }
    };

    for (auto kinematicCursor = editKinematicBody.Execute ();
         auto *body = static_cast<RigidBody3dComponent *> (*kinematicCursor); ++kinematicCursor)
    {
        syncTransform (body);
    }

    for (auto dynamicCursor = editDynamicBody.Execute ();
         auto *body = static_cast<RigidBody3dComponent *> (*dynamicCursor); ++dynamicCursor)
    {
        auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle);
        body->linearVelocity = FromPhysX (pxBody->getLinearVelocity ());
        body->angularVelocity = FromPhysX (pxBody->getAngularVelocity ());

        body->additiveLinearImpulse = Math::Vector3f::ZERO;
        body->additiveAngularImpulse = Math::Vector3f::ZERO;
        syncTransform (body);
    }
}

static physx::PxShapeFlags CalculateShapeFlags (const CollisionShape3dComponent *_shape) noexcept
{
    physx::PxShapeFlags flags = physx::PxShapeFlag::eVISUALIZATION;
    if (_shape->enabled)
    {
        if (_shape->trigger)
        {
            flags |= physx::PxShapeFlag::eTRIGGER_SHAPE;
        }
        else
        {
            flags |= physx::PxShapeFlag::eSIMULATION_SHAPE;
        }

        if (_shape->visibleToWorldQueries)
        {
            flags |= physx::PxShapeFlag::eSCENE_QUERY_SHAPE;
        }
    }

    return flags;
}

static physx::PxGeometryType::Enum ToPxGeometryType (CollisionGeometry3dType _type) noexcept
{
    switch (_type)
    {
    case CollisionGeometry3dType::BOX:
        return physx::PxGeometryType::eBOX;
    case CollisionGeometry3dType::SPHERE:
        return physx::PxGeometryType::eSPHERE;
    case CollisionGeometry3dType::CAPSULE:
        return physx::PxGeometryType::eCAPSULE;
    }

    return physx::PxGeometryType::eINVALID;
}

static bool UpdateShapeGeometry (const CollisionShape3dComponent *_shape, const Math::Vector3f &_worldScale) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    if (pxShape->getGeometryType () != ToPxGeometryType (_shape->geometry.type))
    {
        EMERGENCE_LOG (ERROR,
                       "Physics3d: Unable to update CollisionShape3dComponent geometry, because changing geometry type "
                       "is forbidden!");
        return false;
    }

    switch (_shape->geometry.type)
    {
    case CollisionGeometry3dType::BOX:
        pxShape->setGeometry (physx::PxBoxGeometry {_shape->geometry.boxHalfExtents.x * _worldScale.x,
                                                    _shape->geometry.boxHalfExtents.y * _worldScale.y,
                                                    _shape->geometry.boxHalfExtents.z * _worldScale.z});
        break;

    case CollisionGeometry3dType::SPHERE:
        pxShape->setGeometry (physx::PxSphereGeometry {_shape->geometry.sphereRadius * _worldScale.x});
        break;

    case CollisionGeometry3dType::CAPSULE:
        pxShape->setGeometry (physx::PxCapsuleGeometry {_shape->geometry.capsuleRadius * _worldScale.x,
                                                        _shape->geometry.capsuleHalfHeight * _worldScale.x});
        break;
    }

    return true;
}

static void UpdateShapeLocalPose (const CollisionShape3dComponent *_shape, const Math::Vector3f &_worldScale) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const Math::Transform3d localTransform =
        Math::Transform3d {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, _worldScale} *
        Math::Transform3d {_shape->translation, _shape->rotation, Math::Vector3f::ONE};

    pxShape->setLocalPose (
        physx::PxTransform {ToPhysX (localTransform.translation), ToPhysX (localTransform.rotation)});
}

static void UpdateShapeFilter (const CollisionShape3dComponent *_shape) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const physx::PxFilterData filterData {static_cast<std::uint32_t> (_shape->collisionGroup),
                                          _shape->sendContactEvents ? 1u : 0u, 0u, 0u};
    pxShape->setSimulationFilterData (filterData);
    pxShape->setQueryFilterData (filterData);
}

static void ConstructPxShape (CollisionShape3dComponent *_shape,
                              const PhysXWorld &_pxWorld,
                              const physx::PxMaterial &_pxMaterial,
                              const Math::Vector3f &_worldScale) noexcept
{
    EMERGENCE_ASSERT (!_shape->implementationHandle);
    if ((_shape->geometry.type == CollisionGeometry3dType::SPHERE ||
         _shape->geometry.type == CollisionGeometry3dType::CAPSULE) &&
        (!Math::NearlyEqual (_worldScale.x, _worldScale.y) || !Math::NearlyEqual (_worldScale.y, _worldScale.z)))
    {
        EMERGENCE_LOG (ERROR, "Physics3d: CollisionShape3dComponent's can only work with uniform scale!");
    }

    const physx::PxShapeFlags shapeFlags = CalculateShapeFlags (_shape);
    physx::PxShape *pxShape = nullptr;

    switch (_shape->geometry.type)
    {
    case CollisionGeometry3dType::BOX:
        pxShape =
            _pxWorld.physics->createShape (physx::PxBoxGeometry {_shape->geometry.boxHalfExtents.x * _worldScale.x,
                                                                 _shape->geometry.boxHalfExtents.y * _worldScale.y,
                                                                 _shape->geometry.boxHalfExtents.z * _worldScale.z},
                                           _pxMaterial, true, shapeFlags);
        break;

    case CollisionGeometry3dType::SPHERE:
        pxShape = _pxWorld.physics->createShape (
            physx::PxSphereGeometry {_shape->geometry.sphereRadius * _worldScale.x}, _pxMaterial, true, shapeFlags);
        break;

    case CollisionGeometry3dType::CAPSULE:
        pxShape = _pxWorld.physics->createShape (
            physx::PxCapsuleGeometry {_shape->geometry.capsuleRadius * _worldScale.x,
                                      _shape->geometry.capsuleHalfHeight * _worldScale.x},
            _pxMaterial, true, shapeFlags);
        break;
    }

    _shape->implementationHandle = pxShape;
    pxShape->userData = _shape;

    UpdateShapeLocalPose (_shape, _worldScale);
    UpdateShapeFilter (_shape);
}

static physx::PxFilterFlags PhysicsFilterShader (physx::PxFilterObjectAttributes _attributes0,
                                                 // NOLINTNEXTLINE(performance-unnecessary-value-param): PhysX API.
                                                 physx::PxFilterData _filterData0,
                                                 physx::PxFilterObjectAttributes _attributes1,
                                                 // NOLINTNEXTLINE(performance-unnecessary-value-param): PhysX API.
                                                 physx::PxFilterData _filterData1,
                                                 physx::PxPairFlags &_pairFlags,
                                                 const void *_constantBlock,
                                                 physx::PxU32 /*unused*/)
{
    const auto &collisionMasks = *static_cast<const std::array<std::uint32_t, 32u> *> (_constantBlock);
    const bool collision0to1 = collisionMasks[_filterData0.word0] & (1u << _filterData1.word0);
    const bool collision1to0 = collisionMasks[_filterData1.word0] & (1u << _filterData0.word0);

    if (collision0to1 || collision1to0)
    {
        _pairFlags = physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
        if (physx::PxFilterObjectIsTrigger (_attributes0) || physx::PxFilterObjectIsTrigger (_attributes1))
        {
            // Trigger collisions are always reported.
            _pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
        }
        else
        {
            _pairFlags |= physx::PxPairFlag::eSOLVE_CONTACT;
            if (_filterData0.word1 || _filterData1.word1)
            {
                // Contact reporting enabled for first or second shape.
                _pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
                              physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
            }
        }

        return physx::PxFilterFlag::eDEFAULT;
    }

    return physx::PxFilterFlag::eSUPPRESS;
}

const Memory::UniqueString Checkpoint::STARTED {"Physics3dSimulationStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"Physics3dSimulationFinished"};

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    _pipelineBuilder.AddTask ("Physics3d::RemoveBodies"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, RigidBody3dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Physics3d::RemoveShapes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, CollisionShape3dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Physics3dSimulation");
    _pipelineBuilder.AddCheckpoint (Physics3dSimulation::Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Physics3dSimulation::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask (TaskNames::UPDATE_WORLD).SetExecutor<WorldUpdater> ();
    _pipelineBuilder.AddTask (TaskNames::INITIALIZE_MATERIALS).SetExecutor<MaterialInitializer> ();
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
} // namespace Emergence::Celerity::Physics3dSimulation
