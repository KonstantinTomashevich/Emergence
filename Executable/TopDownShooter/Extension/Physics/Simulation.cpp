#include <cassert>

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysXAccessSingleton.hpp>
#include <Physics/PhysXInternalUtils.hpp>
#include <Physics/PhysXWorld.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

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

#include <SyntaxSugar/BlockCast.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Physics::Simulation
{
namespace TaskNames
{
// TODO: Currently all tasks are sequential and are separated only to make event pipeline work properly.
//       It seems that parallelization is not possible, because of close logical coupling between bodies,
//       shapes and materials. Is there any way to make it better?

static const Memory::UniqueString UPDATE_WORLD {"Physics::UpdateWorld"};
static const Memory::UniqueString PROCESS_TRANSFORM_EVENTS {"Physics::ProcessTransformEvents"};

static const Memory::UniqueString INITIALIZE_MATERIALS {"Physics::InitializeMaterials"};
static const Memory::UniqueString SYNC_MATERIAL_CHANGES {"Physics::SyncMaterialChanges"};
static const Memory::UniqueString APPLY_MATERIAL_DELETION {"Physics::ApplyMaterialDeletion"};

static const Memory::UniqueString INITIALIZE_SHAPES {"Physics::InitializeShapes"};
static const Memory::UniqueString SYNC_SHAPE_CHANGES {"Physics::SyncShapeChanges"};
static const Memory::UniqueString APPLY_SHAPE_DELETION {"Physics::ApplyShapeDeletion"};

static const Memory::UniqueString INITIALIZE_BODIES {"Physics::InitializeBodies"};
static const Memory::UniqueString APPLY_BODY_DELETION {"Physics::ApplyBodyDeletion"};

static const Memory::UniqueString SYNC_BODY_MASSES {"Physics::SyncBodyMasses"};
static const Memory::UniqueString EXECUTE_SIMULATION {"Physics::ExecuteSimulation"};
} // namespace TaskNames

// NOTE: Every task must register physics world singleton modify access if it works directly with PhysX objects.
//       It allows us to be sure that there is no multithreaded PhysX calls.

static physx::PxShapeFlags CalculateShapeFlags (const CollisionShapeComponent *_shape) noexcept;

static physx::PxGeometryType::Enum ToPxGeometryType (CollisionGeometryType _type) noexcept;

static bool UpdateShapeGeometry (const CollisionShapeComponent *_shape, const Math::Vector3f &_worldScale) noexcept;

static void UpdateShapeLocalPose (const CollisionShapeComponent *_shape, const Math::Vector3f &_worldScale) noexcept;

static void UpdateShapeFilter (const CollisionShapeComponent *_shape) noexcept;

static void ConstructPxShape (CollisionShapeComponent *_shape,
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

class WorldUpdater final : public Celerity::TaskExecutorBase<WorldUpdater>
{
public:
    WorldUpdater (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static void EnsurePhysicsWorldReady (const PhysicsWorldSingleton *_physicsWorld) noexcept;

    void UpdateConfiguration (const PhysicsWorldSingleton *_physicsWorld) noexcept;

    static void UpdateCollisionMask (const PhysicsWorldSingleton *_physicsWorld) noexcept;

    static void UpdateRemoteDebugging (const PhysicsWorldSingleton *_physicsWorld) noexcept;

    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Celerity::FetchSequenceQuery fetchConfigurationChangedEvents;
};

WorldUpdater::WorldUpdater (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (PhysicsWorldSingleton)),
      fetchConfigurationChangedEvents (_constructor.MFetchSequence (PhysicsWorldConfigurationChanged))
{
    _constructor.DependOn (Checkpoint::SIMULATION_STARTED);
}

void WorldUpdater::Execute () noexcept
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorldSingleton *> (*physicsWorldCursor);
    EnsurePhysicsWorldReady (physicsWorld);
    UpdateRemoteDebugging (physicsWorld);
    UpdateConfiguration (physicsWorld);
}

void WorldUpdater::EnsurePhysicsWorldReady (const PhysicsWorldSingleton *_physicsWorld) noexcept
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
        pxWorld.dispatcher = physx::PxDefaultCpuDispatcherCreate (_physicsWorld->simulationMaxThreads);

        physx::PxSceneDesc sceneDescriptor {pxWorld.physics->getTolerancesScale ()};
        sceneDescriptor.gravity = {_physicsWorld->gravity.x, _physicsWorld->gravity.y, _physicsWorld->gravity.z};
        sceneDescriptor.cpuDispatcher = pxWorld.dispatcher;
        sceneDescriptor.filterShader = PhysicsFilterShader;

        pxWorld.scene = pxWorld.physics->createScene (sceneDescriptor);
        UpdateCollisionMask (_physicsWorld);
    }
}

void WorldUpdater::UpdateConfiguration (const PhysicsWorldSingleton *_physicsWorld) noexcept
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

void WorldUpdater::UpdateCollisionMask (const PhysicsWorldSingleton *_physicsWorld) noexcept
{
    const auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    pxWorld.scene->setFilterShaderData (_physicsWorld->collisionMasks.data (), sizeof (_physicsWorld->collisionMasks));
}

void WorldUpdater::UpdateRemoteDebugging (const PhysicsWorldSingleton *_physicsWorld) noexcept
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

class TransformEventProcessor final : public Celerity::TaskExecutorBase<TransformEventProcessor>
{
public:
    TransformEventProcessor (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::RemoveValueQuery removeShapeByObjectId;
    Celerity::RemoveValueQuery removeBodyByObjectId;
    Celerity::FetchValueQuery fetchTransformByObjectId;
    Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
};

TransformEventProcessor::TransformEventProcessor (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      removeShapeByObjectId (_constructor.MRemoveValue1F (CollisionShapeComponent, objectId)),
      removeBodyByObjectId (_constructor.MRemoveValue1F (RigidBodyComponent, objectId)),
      fetchTransformByObjectId (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      fetchTransformRemovedEvents (_constructor.MFetchSequence (Transform::Transform3dComponentRemovedFixedEvent))
{
    _constructor.DependOn (TaskNames::UPDATE_WORLD);
}

void TransformEventProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event = static_cast<const Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto transformCursor = fetchTransformByObjectId.Execute (&event->objectId);
        if (*transformCursor)
        {
            // Another transform component was added.
            continue;
        }

        // Delete shapes from objects without transform.
        for (auto shapeCursor = removeShapeByObjectId.Execute (&event->objectId); shapeCursor.ReadConst ();)
        {
            ~shapeCursor;
        }

        // Delete body from object without transform.
        auto bodyCursor = removeBodyByObjectId.Execute (&event->objectId);

        if (bodyCursor.ReadConst ())
        {
            ~bodyCursor;
        }
    }
}

class MaterialInitializer final : public Celerity::TaskExecutorBase<MaterialInitializer>
{
public:
    MaterialInitializer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Celerity::ModifyValueQuery modifyMaterialById;

    Celerity::FetchSequenceQuery fetchMaterialAddedFixedEvents;
    Celerity::FetchSequenceQuery fetchMaterialAddedCustomEvents;
};

MaterialInitializer::MaterialInitializer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (PhysicsWorldSingleton)),
      modifyMaterialById (_constructor.MModifyValue1F (DynamicsMaterial, id)),
      fetchMaterialAddedFixedEvents (_constructor.MFetchSequence (DynamicsMaterialAddedFixedEvent)),
      fetchMaterialAddedCustomEvents (_constructor.MFetchSequence (DynamicsMaterialAddedCustomToFixedEvent))
{
    _constructor.DependOn (TaskNames::PROCESS_TRANSFORM_EVENTS);
}

void MaterialInitializer::Execute () noexcept
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorldSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (Memory::UniqueString _id)
    {
        auto materialCursor = modifyMaterialById.Execute (&_id);
        if (auto *material = static_cast<DynamicsMaterial *> (*materialCursor))
        {
            assert (!material->implementationHandle);

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
         const auto *event = static_cast<const DynamicsMaterialAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->id);
    }

    for (auto eventCursor = fetchMaterialAddedCustomEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialAddedCustomToFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->id);
    }
}

class MaterialChangesSynchronizer final : public Celerity::TaskExecutorBase<MaterialChangesSynchronizer>
{
public:
    MaterialChangesSynchronizer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchValueQuery fetchMaterialById;
    Celerity::FetchSequenceQuery fetchMaterialChangedEvents;
    Celerity::FetchValueQuery fetchShapeByMaterialId;
    Celerity::InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

MaterialChangesSynchronizer::MaterialChangesSynchronizer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchMaterialById (_constructor.MFetchValue1F (DynamicsMaterial, id)),
      fetchMaterialChangedEvents (_constructor.MFetchSequence (DynamicsMaterialChangedEvent)),
      fetchShapeByMaterialId (_constructor.MFetchValue1F (CollisionShapeComponent, materialId)),
      insertBodyMassInvalidatedEvents (_constructor.MInsertShortTerm (RigidBodyComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_MATERIALS);
}

void MaterialChangesSynchronizer::Execute () noexcept
{
    for (auto eventCursor = fetchMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto materialCursor = fetchMaterialById.Execute (&event->id);
        if (const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor))
        {
            auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
            pxMaterial->setStaticFriction (material->staticFriction);
            pxMaterial->setDynamicFriction (material->dynamicFriction);
            pxMaterial->setRestitution (material->restitution);
            pxMaterial->setFlag (physx::PxMaterialFlag::eDISABLE_FRICTION, !material->enableFriction);

            for (auto shapeCursor = fetchShapeByMaterialId.Execute (&material->id);
                 const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor); ++shapeCursor)
            {
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
            }
        }
        else
        {
            // Material is already removed, event was outdated.
        }
    }
}

class MaterialDeleter final : public Celerity::TaskExecutorBase<MaterialDeleter>
{
public:
    MaterialDeleter (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSequenceQuery fetchMaterialRemovedEvents;
    Celerity::RemoveValueQuery removeShapeByMaterialId;
};

MaterialDeleter::MaterialDeleter (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchMaterialRemovedEvents (_constructor.MFetchSequence (DynamicsMaterialRemovedEvent)),
      removeShapeByMaterialId (_constructor.MRemoveValue1F (CollisionShapeComponent, materialId))
{
    _constructor.DependOn (TaskNames::SYNC_MATERIAL_CHANGES);
}

void MaterialDeleter::Execute () noexcept
{
    for (auto eventCursor = fetchMaterialRemovedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialRemovedEvent *> (*eventCursor); ++eventCursor)
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

class ShapeInitializer final : public Celerity::TaskExecutorBase<ShapeInitializer>
{
public:
    ShapeInitializer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSingletonQuery fetchPhysicsWorld;

    Celerity::ModifyValueQuery modifyShapeByShapeId;
    Celerity::FetchValueQuery fetchMaterialById;
    Celerity::FetchValueQuery fetchBodyByObjectId;

    Celerity::FetchValueQuery fetchTransformByObjectId;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

    Celerity::FetchSequenceQuery fetchShapeAddedFixedEvents;
    Celerity::FetchSequenceQuery fetchShapeAddedCustomEvents;
    Celerity::InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeInitializer::ShapeInitializer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (PhysicsWorldSingleton)),

      modifyShapeByShapeId (_constructor.MModifyValue1F (CollisionShapeComponent, shapeId)),
      fetchMaterialById (_constructor.MFetchValue1F (DynamicsMaterial, id)),
      fetchBodyByObjectId (_constructor.MFetchValue1F (RigidBodyComponent, objectId)),

      fetchTransformByObjectId (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeAddedFixedEvents (_constructor.MFetchSequence (CollisionShapeComponentAddedFixedEvent)),
      fetchShapeAddedCustomEvents (_constructor.MFetchSequence (CollisionShapeComponentAddedCustomToFixedEvent)),
      insertBodyMassInvalidatedEvents (_constructor.MInsertShortTerm (RigidBodyComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_MATERIAL_DELETION);
}

void ShapeInitializer::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorldSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (Celerity::UniqueId _shapeId)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&_shapeId);
        auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor);

        if (!shape)
        {
            // Shape is already removed.
            return;
        }

        auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
        const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor);

        if (!material)
        {
            EMERGENCE_LOG (ERROR, "Physics: Unable to find DynamicsMaterial with id ", shape->materialId,
                           "! Shape, that attempts to use this material, will be deleted.");

            ~shapeCursor;
            return;
        }

        const auto *pxMaterial = static_cast<const physx::PxMaterial *> (material->implementationHandle);
        auto transformCursor = fetchTransformByObjectId.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics: Unable to add CollisionShapeComponent to object with id ", shape->objectId,
                           ", because it has no Transform3dComponent!");

            ~shapeCursor;
            return;
        }

        const Math::Vector3f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        ConstructPxShape (shape, pxWorld, *pxMaterial, worldScale);
        auto bodyCursor = fetchBodyByObjectId.Execute (&shape->objectId);

        if (const auto *body = static_cast<const RigidBodyComponent *> (*bodyCursor))
        {
            if (auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle))
            {
                pxBody->attachShape (*static_cast<physx::PxShape *> (shape->implementationHandle));
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
            }
            else
            {
                // Body is not initialized yet. Shape will be added during initialization.
            }
        }
    };

    for (auto eventCursor = fetchShapeAddedFixedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->shapeId);
    }

    for (auto eventCursor = fetchShapeAddedCustomEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->shapeId);
    }
}

class ShapeChangesSynchronizer final : public Celerity::TaskExecutorBase<ShapeChangesSynchronizer>
{
public:
    ShapeChangesSynchronizer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void ApplyShapeMaterialChanges () noexcept;

    void ApplyShapeGeometryChanges () noexcept;

    void ApplyShapeAttributesChanges () noexcept;

    Celerity::ModifySingletonQuery modifyPhysX;

    Celerity::FetchValueQuery fetchShapeByShapeId;
    Celerity::RemoveValueQuery removeShapeByShapeId;
    Celerity::FetchValueQuery fetchMaterialById;

    Celerity::FetchValueQuery fetchTransformById;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

    Celerity::FetchSequenceQuery fetchShapeMaterialChangedEvents;
    Celerity::FetchSequenceQuery fetchShapeGeometryChangedEvents;
    Celerity::FetchSequenceQuery fetchShapeAttributesChangedEvents;

    Celerity::InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeChangesSynchronizer::ShapeChangesSynchronizer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchShapeByShapeId (_constructor.MFetchValue1F (CollisionShapeComponent, shapeId)),
      removeShapeByShapeId (_constructor.MRemoveValue1F (CollisionShapeComponent, shapeId)),
      fetchMaterialById (_constructor.MFetchValue1F (DynamicsMaterial, id)),

      fetchTransformById (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchShapeMaterialChangedEvents (_constructor.MFetchSequence (CollisionShapeComponentMaterialChangedEvent)),
      fetchShapeGeometryChangedEvents (_constructor.MFetchSequence (CollisionShapeComponentGeometryChangedEvent)),
      fetchShapeAttributesChangedEvents (_constructor.MFetchSequence (CollisionShapeComponentAttributesChangedEvent)),
      insertBodyMassInvalidatedEvents (_constructor.MInsertShortTerm (RigidBodyComponentMassInvalidatedEvent))
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
         const auto *event = static_cast<const CollisionShapeComponentMaterialChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShapeComponent *> (shapeCursor.ReadConst ()))
        {
            auto cursor = insertBodyMassInvalidatedEvents.Execute ();
            static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
            auto materialCursor = fetchMaterialById.Execute (&shape->materialId);

            if (const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor))
            {
                auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
                static_cast<physx::PxShape *> (shape->implementationHandle)->setMaterials (&pxMaterial, 1u);
            }
            else
            {
                EMERGENCE_LOG (ERROR, "Physics: Unable to find DynamicsMaterial with id ", shape->materialId,
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
         const auto *event = static_cast<const CollisionShapeComponentGeometryChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = removeShapeByShapeId.Execute (&event->shapeId);
        const auto *shape = static_cast<const CollisionShapeComponent *> (shapeCursor.ReadConst ());

        if (!shape)
        {
            continue;
        }

        auto cursor = insertBodyMassInvalidatedEvents.Execute ();
        static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
        auto transformCursor = fetchTransformById.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics: Unable to update CollisionShapeComponent to object with id ",
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
         const auto *event = static_cast<const CollisionShapeComponentAttributesChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = fetchShapeByShapeId.Execute (&event->shapeId);
        if (const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor))
        {
            static_cast<physx::PxShape *> (shape->implementationHandle)->setFlags (CalculateShapeFlags (shape));
            UpdateShapeFilter (shape);

            auto cursor = insertBodyMassInvalidatedEvents.Execute ();
            static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = shape->objectId;
        }
    }
}

class ShapeDeleter final : public Celerity::TaskExecutorBase<ShapeDeleter>
{
public:
    ShapeDeleter (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchValueQuery fetchBodyByObjectId;
    Celerity::FetchSequenceQuery fetchShapeRemovedEvents;
    Celerity::InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

ShapeDeleter::ShapeDeleter (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchBodyByObjectId (_constructor.MFetchValue1F (RigidBodyComponent, objectId)),
      fetchShapeRemovedEvents (_constructor.MFetchSequence (CollisionShapeComponentRemovedEvent)),
      insertBodyMassInvalidatedEvents (_constructor.MInsertShortTerm (RigidBodyComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::SYNC_SHAPE_CHANGES);
}

void ShapeDeleter::Execute ()
{
    for (auto eventCursor = fetchShapeRemovedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto *pxShape = static_cast<physx::PxShape *> (event->implementationHandle))
        {
            auto bodyCursor = fetchBodyByObjectId.Execute (&event->objectId);
            if (const auto *body = static_cast<const RigidBodyComponent *> (*bodyCursor))
            {
                static_cast<physx::PxRigidBody *> (body->implementationHandle)->detachShape (*pxShape);
                auto cursor = insertBodyMassInvalidatedEvents.Execute ();
                static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = body->objectId;
            }
            else
            {
                // Body was removed too.
            }

            pxShape->release ();
        }
    }
}

class BodyInitializer final : public Celerity::TaskExecutorBase<BodyInitializer>
{
public:
    BodyInitializer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSingletonQuery fetchPhysicsWorld;

    Celerity::ModifyValueQuery modifyBodyByObjectId;
    Celerity::FetchValueQuery fetchShapeByObjectId;
    Celerity::FetchValueQuery fetchTransformByObjectId;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

    Celerity::FetchSequenceQuery fetchBodyAddedFixedEvents;
    Celerity::FetchSequenceQuery fetchBodyAddedCustomEvents;
    Celerity::InsertShortTermQuery insertBodyMassInvalidatedEvents;
};

BodyInitializer::BodyInitializer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (PhysicsWorldSingleton)),

      modifyBodyByObjectId (_constructor.MModifyValue1F (RigidBodyComponent, objectId)),
      fetchShapeByObjectId (_constructor.MFetchValue1F (CollisionShapeComponent, objectId)),
      fetchTransformByObjectId (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchBodyAddedFixedEvents (_constructor.MFetchSequence (RigidBodyComponentAddedFixedEvent)),
      fetchBodyAddedCustomEvents (_constructor.MFetchSequence (RigidBodyComponentAddedCustomToFixedEvent)),
      insertBodyMassInvalidatedEvents (_constructor.MInsertShortTerm (RigidBodyComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_SHAPE_DELETION);
}

void BodyInitializer::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorldSingleton *> (*physicsWorldCursor);
    const auto &pxWorld = block_cast<PhysXWorld> (physicsWorld->implementationBlock);

    auto initialize = [this, &pxWorld] (Celerity::UniqueId _objectId)
    {
        auto bodyCursor = modifyBodyByObjectId.Execute (&_objectId);
        auto *body = static_cast<RigidBodyComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed.
            return;
        }

        auto transformCursor = fetchTransformByObjectId.Execute (&_objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Physics: Unable to initialize RigidBodyComponent on object with id ", body->objectId,
                           ", because it has no Transform3dComponent!");

            ~bodyCursor;
            return;
        }

        const Math::Transform3d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        const physx::PxTransform pxTransform {ToPhysX (logicalTransform.translation),
                                              ToPhysX (logicalTransform.rotation)};

        physx::PxRigidActor *pxActor = nullptr;
        switch (body->type)
        {
        case RigidBodyType::STATIC:
        {
            pxActor = pxWorld.physics->createRigidStatic (pxTransform);
            break;
        }

        case RigidBodyType::KINEMATIC:
        {
            physx::PxRigidDynamic *pxBody = pxWorld.physics->createRigidDynamic (pxTransform);
            pxActor = pxBody;
            pxBody->setRigidBodyFlag (physx::PxRigidBodyFlag::eKINEMATIC, true);
            break;
        }

        case RigidBodyType::DYNAMIC:
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
             const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor); ++shapeCursor)
        {
            pxActor->attachShape (*static_cast<physx::PxShape *> (shape->implementationHandle));
        }

        pxWorld.scene->addActor (*pxActor);
        if (body->type == RigidBodyType::DYNAMIC)
        {
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
        static_cast<RigidBodyComponentMassInvalidatedEvent *> (++cursor)->objectId = body->objectId;
    };

    for (auto eventCursor = fetchBodyAddedFixedEvents.Execute ();
         const auto *event = static_cast<const RigidBodyComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->objectId);
    }

    for (auto eventCursor = fetchBodyAddedCustomEvents.Execute ();
         const auto *event = static_cast<const RigidBodyComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->objectId);
    }
}

class BodyDeleter final : public Celerity::TaskExecutorBase<BodyDeleter>
{
public:
    BodyDeleter (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSequenceQuery fetchBodyRemovedEvents;
};

BodyDeleter::BodyDeleter (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchBodyRemovedEvents (_constructor.MFetchSequence (RigidBodyComponentRemovedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_BODIES);
}

void BodyDeleter::Execute ()
{
    for (auto eventCursor = fetchBodyRemovedEvents.Execute ();
         const auto *event = static_cast<const RigidBodyComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto *pxActor = static_cast<physx::PxRigidActor *> (event->implementationHandle))
        {
            pxActor->release ();
        }
    }
}

class BodyMassSynchronizer final : public Celerity::TaskExecutorBase<BodyMassSynchronizer>
{
public:
    BodyMassSynchronizer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchValueQuery fetchBodyByObjectId;
    Celerity::FetchValueQuery fetchShapeByObjectId;
    Celerity::FetchValueQuery fetchMaterialById;
    Celerity::FetchSequenceQuery fetchBodyMassInvalidationEvents;
};

BodyMassSynchronizer::BodyMassSynchronizer (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchBodyByObjectId (_constructor.MFetchValue1F (RigidBodyComponent, objectId)),
      fetchShapeByObjectId (_constructor.MFetchValue1F (CollisionShapeComponent, objectId)),
      fetchMaterialById (_constructor.MFetchValue1F (DynamicsMaterial, id)),
      fetchBodyMassInvalidationEvents (_constructor.MFetchSequence (RigidBodyComponentMassInvalidatedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_BODY_DELETION);
}

void BodyMassSynchronizer::Execute ()
{
    // Body mass recalculation is a rare task, therefore we can afford to allocate memory dynamically.
    Container::Vector<physx::PxShape *> shapes {heap.GetAllocationGroup ()};
    Container::Vector<float> densities {heap.GetAllocationGroup ()};
    Container::Vector<Celerity::UniqueId> bodyIds {heap.GetAllocationGroup ()};

    // Filter out duplicates to avoid excessive mass recalculations (there're quite expensive).
    for (auto eventCursor = fetchBodyMassInvalidationEvents.Execute ();
         const auto *event = static_cast<const RigidBodyComponentMassInvalidatedEvent *> (*eventCursor); ++eventCursor)
    {
        Container::AddUnique (bodyIds, event->objectId);
    }

    for (Celerity::UniqueId objectId : bodyIds)
    {
        auto bodyCursor = fetchBodyByObjectId.Execute (&objectId);
        const auto *body = static_cast<const RigidBodyComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed, no need to bother with recalculation.
            continue;
        }

        if (body->type != RigidBodyType::DYNAMIC)
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
        uint32_t shapesFound = 0u;

        for (auto shapeCursor = fetchShapeByObjectId.Execute (&objectId);
             const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor); ++shapeCursor)
        {
            auto iterator =
                std::find (shapes.begin (), shapes.end (), static_cast<physx::PxShape *> (shape->implementationHandle));

            if (iterator == shapes.end ())
            {
                // Not all shapes are attached: this case will be processed inside other routine.
                break;
            }

            auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
            const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor);

            if (!material)
            {
                // Shapes can not exist without material. How this happened?
                assert (false);
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

class SimulationExecutor final : public Celerity::TaskExecutorBase<SimulationExecutor>,
                                 public physx::PxSimulationEventCallback
{
public:
    SimulationExecutor (Celerity::TaskConstructor &_constructor) noexcept;

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

    void ExecuteSimulation (const PhysicsWorldSingleton *_physicsWorld, float _timeStep) noexcept;

    void SyncKinematicAndDynamicBodies () noexcept;

    Celerity::ModifySingletonQuery modifyPhysX;
    Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Celerity::FetchSingletonQuery fetchTime;

    Celerity::FetchValueQuery fetchShapeByObjectId;
    Celerity::EditSignalQuery editBodyWithOutsideManipulations;
    Celerity::EditSignalQuery editKinematicBody;
    Celerity::EditSignalQuery editDynamicBody;

    Celerity::FetchValueQuery fetchTransformByObjectId;
    Celerity::EditValueQuery editTransformByObjectId;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

    Celerity::InsertShortTermQuery insertContactFoundEvents;
    Celerity::InsertShortTermQuery insertContactPersistsEvents;
    Celerity::InsertShortTermQuery insertContactLostEvents;

    Celerity::InsertShortTermQuery insertTriggerEnteredEvents;
    Celerity::InsertShortTermQuery insertTriggerExitedEvents;
};

SimulationExecutor::SimulationExecutor (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysX (_constructor.MModifySingleton (PhysXAccessSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (PhysicsWorldSingleton)),
      fetchTime (_constructor.MFetchSingleton (Celerity::TimeSingleton)),

      fetchShapeByObjectId (_constructor.MFetchValue1F (CollisionShapeComponent, objectId)),
      editBodyWithOutsideManipulations (
          _constructor.MEditSignal (RigidBodyComponent, manipulatedOutsideOfSimulation, true)),
      editKinematicBody (_constructor.MEditSignal (RigidBodyComponent, type, RigidBodyType::KINEMATIC)),
      editDynamicBody (_constructor.MEditSignal (RigidBodyComponent, type, RigidBodyType::DYNAMIC)),

      fetchTransformByObjectId (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      editTransformByObjectId (_constructor.MEditValue1F (Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertContactFoundEvents (_constructor.MInsertShortTerm (ContactFoundEvent)),
      insertContactPersistsEvents (_constructor.MInsertShortTerm (ContactPersistsEvent)),
      insertContactLostEvents (_constructor.MInsertShortTerm (ContactLostEvent)),

      insertTriggerEnteredEvents (_constructor.MInsertShortTerm (TriggerEnteredEvent)),
      insertTriggerExitedEvents (_constructor.MInsertShortTerm (TriggerExitedEvent))
{
    _constructor.DependOn (TaskNames::SYNC_BODY_MASSES);
    _constructor.MakeDependencyOf (Checkpoint::SIMULATION_FINISHED);
}

void SimulationExecutor::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const PhysicsWorldSingleton *> (*physicsWorldCursor);

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Celerity::TimeSingleton *> (*timeCursor);

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

    for (uint32_t index = 0u; index < _nbPairs; ++index)
    {
        const physx::PxContactPair &pair = _pairs[index];
        if (pair.flags & (physx::PxContactPairFlag::eREMOVED_SHAPE_0 | physx::PxContactPairFlag::eREMOVED_SHAPE_1))
        {
            // Same as for bodies.
            continue;
        }

        const auto *firstShape = static_cast<const CollisionShapeComponent *> (pair.shapes[0u]->userData);
        const auto *secondShape = static_cast<const CollisionShapeComponent *> (pair.shapes[1u]->userData);
        assert (firstShape->sendContactEvents || secondShape->sendContactEvents);

#define FILL_EVENT                                                                                                     \
    event->firstObjectId = firstShape->objectId;                                                                       \
    event->firstShapeId = firstShape->shapeId;                                                                         \
    event->secondObjectId = secondShape->objectId;                                                                     \
    event->secondShapeId = secondShape->shapeId;

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            auto cursor = insertContactFoundEvents.Execute ();
            auto *event = static_cast<ContactFoundEvent *> (++cursor);
            FILL_EVENT
            event->initialContact = pair.flags & physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH;
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            auto cursor = insertContactPersistsEvents.Execute ();
            auto *event = static_cast<ContactPersistsEvent *> (++cursor);
            FILL_EVENT
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            auto cursor = insertContactLostEvents.Execute ();
            auto *event = static_cast<ContactLostEvent *> (++cursor);
            FILL_EVENT
            event->lastContact = pair.flags & physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH;
        }

#undef FILL_EVENT
    }
}

void SimulationExecutor::onTrigger (physx::PxTriggerPair *_pairs, physx::PxU32 _count)
{
    for (uint32_t index = 0u; index < _count; ++index)
    {
        const physx::PxTriggerPair &pair = _pairs[index];
        if (pair.flags &
            (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
        {
            // There is no way to send correct event after shapes or bodies were removed.
            // User logic should listen to removal events to be aware of such situations.
            continue;
        }

        const auto *triggerShape = static_cast<const CollisionShapeComponent *> (pair.triggerShape->userData);
        const auto *intruderShape = static_cast<const CollisionShapeComponent *> (pair.otherShape->userData);

#define FILL_EVENT                                                                                                     \
    event->triggerObjectId = triggerShape->objectId;                                                                   \
    event->triggerShapeId = triggerShape->shapeId;                                                                     \
    event->intruderObjectId = intruderShape->objectId;                                                                 \
    event->intruderShapeId = intruderShape->shapeId;

        if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            auto cursor = insertTriggerEnteredEvents.Execute ();
            auto *event = static_cast<TriggerEnteredEvent *> (++cursor);
            FILL_EVENT
        }
        else
        {
            assert (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
            auto cursor = insertTriggerExitedEvents.Execute ();
            auto *event = static_cast<TriggerExitedEvent *> (++cursor);
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
         auto *body = static_cast<RigidBodyComponent *> (*bodyCursor); ++bodyCursor)
    {
        auto *pxActor = static_cast<physx::PxRigidActor *> (body->implementationHandle);
        pxActor->setActorFlag (physx::PxActorFlag::eDISABLE_GRAVITY, !body->affectedByGravity);

        auto transformCursor = fetchTransformByObjectId.Execute (&body->objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            // Transformless body must've been removed by other routines.
            assert (false);
            continue;
        }

        const Math::Transform3d &logicalTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        pxActor->setGlobalPose ({ToPhysX (logicalTransform.translation), ToPhysX (logicalTransform.rotation)});

        if (body->type == RigidBodyType::DYNAMIC)
        {
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
             const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor); ++shapeCursor)
        {
            UpdateShapeGeometry (shape, logicalTransform.scale);
            UpdateShapeLocalPose (shape, logicalTransform.scale);
        }
    }
}

void SimulationExecutor::UpdateKinematicTargets (float _timeStep) noexcept
{
    for (auto kinematicCursor = editKinematicBody.Execute ();
         auto *body = static_cast<RigidBodyComponent *> (*kinematicCursor); ++kinematicCursor)
    {
        auto *pxBody = static_cast<physx::PxRigidDynamic *> (body->implementationHandle);
        physx::PxTransform target = pxBody->getGlobalPose ();

        target.p += ToPhysX (body->linearVelocity * _timeStep);
        target.q *= ToPhysX (Math::Quaternion {body->angularVelocity * _timeStep});
        target.q.normalize ();

        pxBody->setKinematicTarget (target);
    }
}

void SimulationExecutor::ExecuteSimulation (const PhysicsWorldSingleton *_physicsWorld, float _timeStep) noexcept
{
    const auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    pxWorld.scene->setSimulationEventCallback (this);
    // TODO: Make use of scratch buffer?
    pxWorld.scene->simulate (_timeStep);
    pxWorld.scene->fetchResults (true);
}

void SimulationExecutor::SyncKinematicAndDynamicBodies () noexcept
{
    auto syncTransform = [this] (RigidBodyComponent *_body)
    {
        auto *pxBody = static_cast<physx::PxRigidBody *> (_body->implementationHandle);
        auto transformCursor = editTransformByObjectId.Execute (&_body->objectId);

        if (auto *transform = static_cast<Transform::Transform3dComponent *> (*transformCursor))
        {
            const physx::PxTransform &pxTransform = pxBody->getGlobalPose ();
            const Math::Vector3f &scale = transform->GetLogicalLocalTransform ().scale;

            // TODO: Currently, we assume that non-static bodies are attached to transform root elements only.
            transform->SetLogicalLocalTransform ({FromPhysX (pxTransform.p), FromPhysX (pxTransform.q), scale});
        }
    };

    for (auto kinematicCursor = editKinematicBody.Execute ();
         auto *body = static_cast<RigidBodyComponent *> (*kinematicCursor); ++kinematicCursor)
    {
        syncTransform (body);
    }

    for (auto dynamicCursor = editDynamicBody.Execute ();
         auto *body = static_cast<RigidBodyComponent *> (*dynamicCursor); ++dynamicCursor)
    {
        auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle);
        body->linearVelocity = FromPhysX (pxBody->getLinearVelocity ());
        body->angularVelocity = FromPhysX (pxBody->getAngularVelocity ());

        body->additiveLinearImpulse = Math::Vector3f::ZERO;
        body->additiveAngularImpulse = Math::Vector3f::ZERO;
        syncTransform (body);
    }
}

static physx::PxShapeFlags CalculateShapeFlags (const CollisionShapeComponent *_shape) noexcept
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

static physx::PxGeometryType::Enum ToPxGeometryType (CollisionGeometryType _type) noexcept
{
    switch (_type)
    {
    case CollisionGeometryType::BOX:
        return physx::PxGeometryType::eBOX;
    case CollisionGeometryType::SPHERE:
        return physx::PxGeometryType::eSPHERE;
    case CollisionGeometryType::CAPSULE:
        return physx::PxGeometryType::eCAPSULE;
    }

    return physx::PxGeometryType::eINVALID;
}

static bool UpdateShapeGeometry (const CollisionShapeComponent *_shape, const Math::Vector3f &_worldScale) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    if (pxShape->getGeometryType () != ToPxGeometryType (_shape->geometry.type))
    {
        EMERGENCE_LOG (
            ERROR,
            "Physics: Unable to update CollisionShapeComponent geometry, because changing geometry type is forbidden!");
        return false;
    }

    switch (_shape->geometry.type)
    {
    case CollisionGeometryType::BOX:
        pxShape->setGeometry (physx::PxBoxGeometry {_shape->geometry.boxHalfExtents.x * _worldScale.x,
                                                    _shape->geometry.boxHalfExtents.y * _worldScale.y,
                                                    _shape->geometry.boxHalfExtents.z * _worldScale.z});
        break;

    case CollisionGeometryType::SPHERE:
        pxShape->setGeometry (physx::PxSphereGeometry {_shape->geometry.sphereRadius * _worldScale.x});
        break;

    case CollisionGeometryType::CAPSULE:
        pxShape->setGeometry (physx::PxCapsuleGeometry {_shape->geometry.capsuleRadius * _worldScale.x,
                                                        _shape->geometry.capsuleHalfHeight * _worldScale.x});
        break;
    }

    return true;
}

static void UpdateShapeLocalPose (const CollisionShapeComponent *_shape, const Math::Vector3f &_worldScale) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const Math::Transform3d localTransform =
        Math::Transform3d {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, _worldScale} *
        Math::Transform3d {_shape->translation, _shape->rotation, Math::Vector3f::ONE};

    pxShape->setLocalPose (
        physx::PxTransform {ToPhysX (localTransform.translation), ToPhysX (localTransform.rotation)});
}

static void UpdateShapeFilter (const CollisionShapeComponent *_shape) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const physx::PxFilterData filterData {static_cast<uint32_t> (_shape->collisionGroup),
                                          _shape->sendContactEvents ? 1u : 0u, 0u, 0u};
    pxShape->setSimulationFilterData (filterData);
    pxShape->setQueryFilterData (filterData);
}

static void ConstructPxShape (CollisionShapeComponent *_shape,
                              const PhysXWorld &_pxWorld,
                              const physx::PxMaterial &_pxMaterial,
                              const Math::Vector3f &_worldScale) noexcept
{
    assert (!_shape->implementationHandle);
    if ((_shape->geometry.type == CollisionGeometryType::SPHERE ||
         _shape->geometry.type == CollisionGeometryType::CAPSULE) &&
        (!Math::NearlyEqual (_worldScale.x, _worldScale.y) || !Math::NearlyEqual (_worldScale.y, _worldScale.z)))
    {
        EMERGENCE_LOG (ERROR, "Physics: CollisionShapeComponent's can only work with uniform scale!");
    }

    const physx::PxShapeFlags shapeFlags = CalculateShapeFlags (_shape);
    physx::PxShape *pxShape = nullptr;

    switch (_shape->geometry.type)
    {
    case CollisionGeometryType::BOX:
        pxShape =
            _pxWorld.physics->createShape (physx::PxBoxGeometry {_shape->geometry.boxHalfExtents.x * _worldScale.x,
                                                                 _shape->geometry.boxHalfExtents.y * _worldScale.y,
                                                                 _shape->geometry.boxHalfExtents.z * _worldScale.z},
                                           _pxMaterial, true, shapeFlags);
        break;

    case CollisionGeometryType::SPHERE:
        pxShape = _pxWorld.physics->createShape (
            physx::PxSphereGeometry {_shape->geometry.sphereRadius * _worldScale.x}, _pxMaterial, true, shapeFlags);
        break;

    case CollisionGeometryType::CAPSULE:
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
                                                 physx::PxFilterData _filterData0,
                                                 physx::PxFilterObjectAttributes _attributes1,
                                                 physx::PxFilterData _filterData1,
                                                 physx::PxPairFlags &_pairFlags,
                                                 const void *_constantBlock,
                                                 physx::PxU32 /*unused*/)
{
    const auto &collisionMasks = *static_cast<const std::array<uint32_t, 32u> *> (_constantBlock);
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

const Memory::UniqueString Checkpoint::SIMULATION_STARTED {"PhysicsSimulationStarted"};
const Memory::UniqueString Checkpoint::SIMULATION_FINISHED {"PhysicsSimulationFinished"};

void AddToFixedUpdate (Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::UPDATE_WORLD).SetExecutor<WorldUpdater> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_TRANSFORM_EVENTS).SetExecutor<TransformEventProcessor> ();

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
} // namespace Emergence::Physics::Simulation
