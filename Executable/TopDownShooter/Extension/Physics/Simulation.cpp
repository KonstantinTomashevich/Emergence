#include <cassert>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysXWorld.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

#include <PxMaterial.h>
#include <PxPhysicsVersion.h>
#include <PxRigidBody.h>
#include <PxShape.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxRigidBodyExt.h>
#include <pvd/PxPvdTransport.h>

#include <SyntaxSugar/BlockCast.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Physics::Simulation
{
// TODO: This macro might be useful for other systems too.
#ifdef NDEBUG
#    define ENSURE_UNIQUENESS(...)
#else
#    define ENSURE_UNIQUENESS(Cursor, ...)                                                                             \
        if (!*++Cursor)                                                                                                \
        {                                                                                                              \
            EMERGENCE_LOG (ERROR, __VA_ARGS__);                                                                        \
        }
#endif

#define ENSURE_BODY_UNIQUENESS                                                                                         \
    ENSURE_UNIQUENESS (bodyCursor, "PhysicsSimulationExecutor: Object with id ", body->objectId,                       \
                       " has multiple RigidBodyComponent's!")

#define ENSURE_MATERIAL_UNIQUENESS                                                                                     \
    ENSURE_UNIQUENESS (materialCursor, "PhysicsSimulationExecutor: There are several DynamicsMaterial's with id ",     \
                       material->id, "!")

#define ENSURE_SHAPE_UNIQUENESS                                                                                        \
    ENSURE_UNIQUENESS (shapeCursor,                                                                                    \
                       "PhysicsSimulationExecutor: There are several CollisionShapeComponent's with shapeId ",         \
                       shape->shapeId, "!")

#define ENSURE_TRANSFORM_UNIQUENESS                                                                                    \
    ENSURE_UNIQUENESS (transformCursor, "PhysicsSimulationExecutor: Object with id ", transform->GetObjectId (),       \
                       " has multiple Transform3dComponent's!")

class PhysicsSimulationExecutor final : public Celerity::TaskExecutorBase<PhysicsSimulationExecutor>
{
public:
    PhysicsSimulationExecutor (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static void EnsurePhysicsWorldReady (PhysicsWorldSingleton *_physicsWorld) noexcept;

    static void UpdateGravity (PhysicsWorldSingleton *_physicsWorld) noexcept;

    static void UpdateRemoteDebugging (PhysicsWorldSingleton *_physicsWorld) noexcept;

    void CleanupAfterTransformRemoval () noexcept;

    void InitializeMaterials (PhysicsWorldSingleton *_physicsWorld) noexcept;

    void ApplyMaterialChanges () noexcept;

    void ApplyMaterialDestruction () noexcept;

    void InitializeShapes (PhysicsWorldSingleton *_physicsWorld) noexcept;

    void ApplyShapeMaterialChanges () noexcept;

    void ApplyShapeGeometryChanges () noexcept;

    void ApplyShapeAttributesChanges () noexcept;

    void DetachRemovedShapes () noexcept;

    void RecalculateBodyMasses () noexcept;

    static physx::PxShapeFlags CalculateShapeFlags (const CollisionShapeComponent *_shape) noexcept;

    static physx::PxGeometryType::Enum ToPxGeometryType (CollisionGeometryType _type) noexcept;

    static void UpdateShapeLocalPose (CollisionShapeComponent *_shape, const Math::Vector3f &_worldScale) noexcept;

    static void UpdateShapeFilter (CollisionShapeComponent *_shape) noexcept;

    static void ConstructPxShape (CollisionShapeComponent *_shape,
                                  PhysXWorld &_pxWorld,
                                  const physx::PxMaterial &_pxMaterial,
                                  const Math::Vector3f &_worldScale) noexcept;

    Celerity::ModifySingletonQuery modifyPhysicsWorld;

    Celerity::FetchValueQuery fetchMaterialById;
    Celerity::ModifyValueQuery modifyMaterialById;

    Celerity::FetchSequenceQuery fetchMaterialAddedEvents;
    Celerity::FetchSequenceQuery fetchMaterialChangedEvents;
    Celerity::FetchSequenceQuery fetchMaterialRemovedEvents;

    Celerity::ModifyValueQuery modifyShapeByShapeId;
    Celerity::FetchValueQuery fetchShapeByObjectId;
    Celerity::ModifyValueQuery modifyShapeByObjectId;
    Celerity::ModifyValueQuery modifyShapeByMaterialId;

    Celerity::FetchSequenceQuery fetchShapeAddedEvents;
    Celerity::FetchSequenceQuery fetchShapeMaterialChangedEvents;
    Celerity::FetchSequenceQuery fetchShapeGeometryChangedEvents;
    Celerity::FetchSequenceQuery fetchShapeAttributesChangedEvents;
    Celerity::FetchSequenceQuery fetchShapeRemovedEvents;

    Celerity::ModifyValueQuery modifyBodyByObjectId;

    Celerity::FetchValueQuery fetchTransformByObjectId;
    Celerity::ModifyValueQuery modifyTransformByObjectId;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

    Celerity::FetchSequenceQuery fetchTransformRemovedEvents;

    /// \details Mass invalidation occurs only when shapes are added/changed/removed or materials are changed.
    ///          Because several shapes on one body might be after during same frame, we collect all object
    ///          ids in this array and then schedule recalculation.
    ///
    ///          We cache this vector to avoid unnecessary allocations when some shapes are changed by gameplay
    ///          logic every frame. For example: fire pool grows until its timer runs out.
    Container::Vector<Celerity::UniqueId> objectsWithInvalidMasses {heap.GetAllocationGroup ()};
};

PhysicsSimulationExecutor::PhysicsSimulationExecutor (Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysicsWorld (_constructor.ModifySingleton (PhysicsWorldSingleton::Reflect ().mapping)),

      fetchMaterialById (
          _constructor.FetchValue (DynamicsMaterial::Reflect ().mapping, {DynamicsMaterial::Reflect ().id})),
      modifyMaterialById (
          _constructor.ModifyValue (DynamicsMaterial::Reflect ().mapping, {DynamicsMaterial::Reflect ().id})),

      fetchMaterialAddedEvents (_constructor.FetchSequence (DynamicsMaterialAddedEvent::Reflect ().mapping)),
      fetchMaterialChangedEvents (_constructor.FetchSequence (DynamicsMaterialChangedEvent::Reflect ().mapping)),
      fetchMaterialRemovedEvents (_constructor.FetchSequence (DynamicsMaterialRemovedEvent::Reflect ().mapping)),

      modifyShapeByShapeId (_constructor.ModifyValue (CollisionShapeComponent::Reflect ().mapping,
                                                      {CollisionShapeComponent::Reflect ().shapeId})),
      fetchShapeByObjectId (_constructor.FetchValue (CollisionShapeComponent::Reflect ().mapping,
                                                     {CollisionShapeComponent::Reflect ().objectId})),
      modifyShapeByObjectId (_constructor.ModifyValue (CollisionShapeComponent::Reflect ().mapping,
                                                       {CollisionShapeComponent::Reflect ().objectId})),
      modifyShapeByMaterialId (_constructor.ModifyValue (CollisionShapeComponent::Reflect ().mapping,
                                                         {CollisionShapeComponent::Reflect ().materialId})),

      fetchShapeAddedEvents (_constructor.FetchSequence (CollisionShapeComponentAddedEvent::Reflect ().mapping)),
      fetchShapeMaterialChangedEvents (
          _constructor.FetchSequence (CollisionShapeComponentMaterialChangedEvent::Reflect ().mapping)),
      fetchShapeGeometryChangedEvents (
          _constructor.FetchSequence (CollisionShapeComponentGeometryChangedEvent::Reflect ().mapping)),
      fetchShapeAttributesChangedEvents (
          _constructor.FetchSequence (CollisionShapeComponentAttributesChangedEvent::Reflect ().mapping)),
      fetchShapeRemovedEvents (_constructor.FetchSequence (CollisionShapeComponentRemovedEvent::Reflect ().mapping)),

      modifyBodyByObjectId (
          _constructor.ModifyValue (RigidBodyComponent::Reflect ().mapping, {RigidBodyComponent::Reflect ().objectId})),

      fetchTransformByObjectId (_constructor.FetchValue (Transform::Transform3dComponent::Reflect ().mapping,
                                                         {Transform::Transform3dComponent::Reflect ().objectId})),
      modifyTransformByObjectId (_constructor.ModifyValue (Transform::Transform3dComponent::Reflect ().mapping,
                                                           {Transform::Transform3dComponent::Reflect ().objectId})),
      transformWorldAccessor (_constructor),

      fetchTransformRemovedEvents (
          _constructor.FetchSequence (Transform::Transform3dComponentRemovedFixedEvent::Reflect ().mapping))
{
    _constructor.DependOn (Checkpoint::SIMULATION_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::SIMULATION_FINISHED);
}

void PhysicsSimulationExecutor::Execute () noexcept
{
    auto physicsWorldCursor = modifyPhysicsWorld.Execute ();
    auto *physicsWorld = static_cast<PhysicsWorldSingleton *> (*physicsWorldCursor);

    if (!physicsWorld)
    {
        return;
    }

    EnsurePhysicsWorldReady (physicsWorld);
    UpdateGravity (physicsWorld);
    UpdateRemoteDebugging (physicsWorld);

    CleanupAfterTransformRemoval ();

    InitializeMaterials (physicsWorld);
    ApplyMaterialChanges ();
    ApplyMaterialDestruction ();

    InitializeShapes (physicsWorld);
    ApplyShapeMaterialChanges ();
    ApplyShapeGeometryChanges ();
    ApplyShapeAttributesChanges ();
    DetachRemovedShapes ();

    // TODO: Rigid body addition.
    // TODO: Rigid body changes.
    // TODO: Rigid body removal.

    RecalculateBodyMasses ();

    // TODO: Game -> PhysX transform and velocity sync.
    // TODO: Simulation.
    // TODO: PhysX -> Game transform and velocity sync.
}

void PhysicsSimulationExecutor::EnsurePhysicsWorldReady (PhysicsWorldSingleton *_physicsWorld) noexcept
{
    auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
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
        sceneDescriptor.filterShader = physx::PxDefaultSimulationFilterShader;
        pxWorld.scene = pxWorld.physics->createScene (sceneDescriptor);
    }
}

void PhysicsSimulationExecutor::UpdateGravity (PhysicsWorldSingleton *_physicsWorld) noexcept
{
    auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    const physx::PxVec3 &gravity = pxWorld.scene->getGravity ();

    if (!Math::NearlyEqual (gravity.x, _physicsWorld->gravity.x) ||
        !Math::NearlyEqual (gravity.y, _physicsWorld->gravity.y) ||
        !Math::NearlyEqual (gravity.z, _physicsWorld->gravity.z))
    {
        pxWorld.scene->setGravity ({_physicsWorld->gravity.x, _physicsWorld->gravity.y, _physicsWorld->gravity.z});
    }
}

void PhysicsSimulationExecutor::UpdateRemoteDebugging (PhysicsWorldSingleton *_physicsWorld) noexcept
{
    auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
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

void PhysicsSimulationExecutor::CleanupAfterTransformRemoval () noexcept
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
        for (auto shapeCursor = modifyShapeByObjectId.Execute (&event->objectId); *shapeCursor;)
        {
            ~shapeCursor;
        }

        // Delete body from object without transform.
        auto bodyCursor = modifyBodyByObjectId.Execute (&event->objectId);

        if (*bodyCursor)
        {
            ~bodyCursor;
        }
    }
}

void PhysicsSimulationExecutor::InitializeMaterials (PhysicsWorldSingleton *_physicsWorld) noexcept
{
    auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    for (auto eventCursor = fetchMaterialAddedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto materialCursor = modifyMaterialById.Execute (&event->id);
        if (auto *material = static_cast<DynamicsMaterial *> (*materialCursor))
        {
            assert (!material->implementationHandle);

            physx::PxMaterial *pxMaterial = pxWorld.physics->createMaterial (
                material->staticFriction, material->dynamicFriction, material->restitution);
            pxMaterial->setFlag (physx::PxMaterialFlag::eDISABLE_FRICTION, !material->enableFriction);
            material->implementationHandle = pxMaterial;
            ENSURE_MATERIAL_UNIQUENESS
        }
        else
        {
            // Material is already removed, event was outdated.
        }
    }
}

void PhysicsSimulationExecutor::ApplyMaterialChanges () noexcept
{
    for (auto eventCursor = fetchMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto materialCursor = modifyMaterialById.Execute (&event->id);
        if (auto *material = static_cast<DynamicsMaterial *> (*materialCursor))
        {
            auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
            pxMaterial->setStaticFriction (material->staticFriction);
            pxMaterial->setDynamicFriction (material->dynamicFriction);
            pxMaterial->setRestitution (material->restitution);
            pxMaterial->setFlag (physx::PxMaterialFlag::eDISABLE_FRICTION, !material->enableFriction);

            auto shapeCursor = modifyShapeByMaterialId.Execute (&material->id);
            while (auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor))
            {
                Container::AddUnique (objectsWithInvalidMasses, shape->objectId);
            }
        }
        else
        {
            // Material is already removed, event was outdated.
        }
    }
}

void PhysicsSimulationExecutor::ApplyMaterialDestruction () noexcept
{
    for (auto eventCursor = fetchMaterialRemovedEvents.Execute ();
         const auto *event = static_cast<const DynamicsMaterialRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        for (auto shapeCursor = modifyShapeByMaterialId.Execute (&event->id); *shapeCursor;)
        {
            // Shapes can not exist without material, therefore we are cleaning them up.
            ~shapeCursor;
        }
    }
}

void PhysicsSimulationExecutor::InitializeShapes (PhysicsWorldSingleton *_physicsWorld) noexcept
{
    auto &pxWorld = block_cast<PhysXWorld> (_physicsWorld->implementationBlock);
    for (auto eventCursor = fetchShapeAddedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&event->shapeId);
        auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor);

        if (!shape)
        {
            // Shape is already removed.
            continue;
        }

        auto materialCursor = fetchMaterialById.Execute (&shape->materialId);
        const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor);

        if (!material)
        {
            EMERGENCE_LOG (ERROR, "PhysicsSimulationExecutor: Unable to find DynamicsMaterial with id ",
                           shape->materialId, "! Shape, that attempts to use this material, will be deleted.");

            ~shapeCursor;
            continue;
        }

        const auto *pxMaterial = static_cast<const physx::PxMaterial *> (material->implementationHandle);
        auto transformCursor = fetchTransformByObjectId.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "PhysicsSimulationExecutor: Unable to add CollisionShapeComponent to object with id ",
                           shape->objectId, ", because it has no Transform3dComponent!");

            ~shapeCursor;
            continue;
        }

        const Math::Vector3f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        ConstructPxShape (shape, pxWorld, *pxMaterial, worldScale);
        auto bodyCursor = modifyBodyByObjectId.Execute (&shape->objectId);

        if (auto *body = static_cast<RigidBodyComponent *> (*bodyCursor))
        {
            if (auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle))
            {
                pxBody->attachShape (*static_cast<physx::PxShape *> (shape->implementationHandle));
                Container::AddUnique (objectsWithInvalidMasses, shape->objectId);
            }
            else
            {
                // Body is not initialized yet. Shape will be added during initialization.
            }

            ENSURE_BODY_UNIQUENESS
        }

        ENSURE_TRANSFORM_UNIQUENESS
        ENSURE_MATERIAL_UNIQUENESS
        ENSURE_SHAPE_UNIQUENESS
    }
}

void PhysicsSimulationExecutor::ApplyShapeMaterialChanges () noexcept
{
    for (auto eventCursor = fetchShapeMaterialChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentMaterialChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&event->shapeId);
        if (auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor))
        {
            Container::AddUnique (objectsWithInvalidMasses, shape->objectId);
            auto materialCursor = fetchMaterialById.Execute (&shape->materialId);

            if (const auto *material = static_cast<const DynamicsMaterial *> (*materialCursor))
            {
                auto *pxMaterial = static_cast<physx::PxMaterial *> (material->implementationHandle);
                static_cast<physx::PxShape *> (shape->implementationHandle)->setMaterials (&pxMaterial, 1u);
                ENSURE_MATERIAL_UNIQUENESS
            }
            else
            {
                EMERGENCE_LOG (ERROR, "PhysicsSimulationExecutor: Unable to find DynamicsMaterial with id ",
                               shape->materialId, "! Shape, that attempts to use this material, will be deleted.");

                ~shapeCursor;
                continue;
            }

            ENSURE_SHAPE_UNIQUENESS
        }
    }
}

void PhysicsSimulationExecutor::ApplyShapeGeometryChanges () noexcept
{
    for (auto eventCursor = fetchShapeGeometryChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentGeometryChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&event->shapeId);
        auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor);

        if (!shape)
        {
            continue;
        }

        Container::AddUnique (objectsWithInvalidMasses, shape->objectId);
        auto *pxShape = static_cast<physx::PxShape *> (shape->implementationHandle);
        auto transformCursor = fetchTransformByObjectId.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR,
                           "PhysicsSimulationExecutor: Unable to update CollisionShapeComponent to object with id ",
                           shape->objectId, ", because it has no Transform3dComponent!");

            ~shapeCursor;
            continue;
        }

        const Math::Vector3f worldScale = transform->GetLogicalWorldTransform (transformWorldAccessor).scale;
        assert (pxShape->getGeometryType () == ToPxGeometryType (shape->geometry.type));

        switch (shape->geometry.type)
        {
        case CollisionGeometryType::BOX:
            pxShape->setGeometry (physx::PxBoxGeometry {shape->geometry.boxHalfExtents.x * worldScale.x,
                                                        shape->geometry.boxHalfExtents.y * worldScale.y,
                                                        shape->geometry.boxHalfExtents.z * worldScale.z});
            break;

        case CollisionGeometryType::SPHERE:
            pxShape->setGeometry (physx::PxSphereGeometry {shape->geometry.sphereRadius * worldScale.x});
            break;

        case CollisionGeometryType::CAPSULE:
            pxShape->setGeometry (physx::PxCapsuleGeometry {shape->geometry.capsuleRadius * worldScale.x,
                                                            shape->geometry.capsuleHalfHeight * worldScale.x});
            break;
        }

        UpdateShapeLocalPose (shape, worldScale);
        ENSURE_TRANSFORM_UNIQUENESS
        ENSURE_SHAPE_UNIQUENESS
    }
}

void PhysicsSimulationExecutor::ApplyShapeAttributesChanges () noexcept
{
    for (auto eventCursor = fetchShapeGeometryChangedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentGeometryChangedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto shapeCursor = modifyShapeByShapeId.Execute (&event->shapeId);
        if (auto *shape = static_cast<CollisionShapeComponent *> (*shapeCursor))
        {
            static_cast<physx::PxShape *> (shape->implementationHandle)->setFlags (CalculateShapeFlags (shape));
            UpdateShapeFilter (shape);
            Container::AddUnique (objectsWithInvalidMasses, shape->objectId);
            ENSURE_SHAPE_UNIQUENESS
        }
    }
}

void PhysicsSimulationExecutor::DetachRemovedShapes () noexcept
{
    for (auto eventCursor = fetchShapeRemovedEvents.Execute ();
         const auto *event = static_cast<const CollisionShapeComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        auto bodyCursor = modifyBodyByObjectId.Execute (&event->objectId);
        if (auto *body = static_cast<RigidBodyComponent *> (*bodyCursor))
        {
            static_cast<physx::PxRigidBody *> (body->implementationHandle)
                ->detachShape (*static_cast<physx::PxShape *> (const_cast<void *> (event->implementationHandle)));
            Container::AddUnique (objectsWithInvalidMasses, body->objectId);
            ENSURE_BODY_UNIQUENESS
        }
        else
        {
            // Body was removed too.
        }
    }
}

void PhysicsSimulationExecutor::RecalculateBodyMasses () noexcept
{
    // Body mass recalculation is a rare task, therefore we can afford to allocate memory dynamically.
    Container::Vector<physx::PxShape *> shapes {heap.GetAllocationGroup ()};
    Container::Vector<float> densities {heap.GetAllocationGroup ()};

    // Count of recalculation requests per frame depends on game logic and may vary greatly during
    // different phases of gameplay. Therefore, we always shrink the vector to maintain optimal size.
    objectsWithInvalidMasses.shrink_to_fit ();

    for (Celerity::UniqueId objectId : objectsWithInvalidMasses)
    {
        auto bodyCursor = modifyBodyByObjectId.Execute (&objectId);
        auto *body = static_cast<RigidBodyComponent *> (*bodyCursor);

        if (!body)
        {
            // Body is already removed, no need to bother with recalculation.
            continue;
        }

        auto *pxBody = static_cast<physx::PxRigidBody *> (body->implementationHandle);
        shapes.resize (pxBody->getNbShapes ());
        pxBody->getShapes (shapes.data (), shapes.size ());

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
        auto shapeCursor = fetchShapeByObjectId.Execute (&objectId);

        while (const auto *shape = static_cast<const CollisionShapeComponent *> (*shapeCursor))
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
            ENSURE_MATERIAL_UNIQUENESS
        }

        if (shapesFound == shapes.size ())
        {
            physx::PxRigidBodyExt::updateMassAndInertia (*pxBody, densities.data (), densities.size ());
        }
        else
        {
            // Body is not ready for recalculation. It will be called from other routine when body will be ready.
        }

        ENSURE_BODY_UNIQUENESS
    }
}

physx::PxShapeFlags PhysicsSimulationExecutor::CalculateShapeFlags (const CollisionShapeComponent *_shape) noexcept
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

physx::PxGeometryType::Enum PhysicsSimulationExecutor::ToPxGeometryType (CollisionGeometryType _type) noexcept
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

void PhysicsSimulationExecutor::UpdateShapeLocalPose (CollisionShapeComponent *_shape,
                                                      const Math::Vector3f &_worldScale) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const Math::Transform3d localTransform =
        Math::Transform3d {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, _worldScale} *
        Math::Transform3d {_shape->translation, _shape->rotation, Math::Vector3f::ONE};

    pxShape->setLocalPose (physx::PxTransform {
        {localTransform.translation.x, localTransform.translation.y, localTransform.translation.z},
        {localTransform.rotation.x, localTransform.rotation.y, localTransform.rotation.z, localTransform.rotation.w}});
}

void PhysicsSimulationExecutor::UpdateShapeFilter (CollisionShapeComponent *_shape) noexcept
{
    auto *pxShape = static_cast<physx::PxShape *> (_shape->implementationHandle);
    const physx::PxFilterData filterData {static_cast<uint32_t> (_shape->collisionGroup), 0u, 0u, 0u};
    pxShape->setSimulationFilterData (filterData);
    pxShape->setQueryFilterData (filterData);
}

void PhysicsSimulationExecutor::ConstructPxShape (CollisionShapeComponent *_shape,
                                                  PhysXWorld &_pxWorld,
                                                  const physx::PxMaterial &_pxMaterial,
                                                  const Math::Vector3f &_worldScale) noexcept
{
    assert (!_shape->implementationHandle);
    if ((_shape->geometry.type == CollisionGeometryType::SPHERE ||
         _shape->geometry.type == CollisionGeometryType::CAPSULE) &&
        (!Math::NearlyEqual (_worldScale.x, _worldScale.y) || !Math::NearlyEqual (_worldScale.y, _worldScale.z)))
    {
        EMERGENCE_LOG (ERROR, "PhysicsSimulationExecutor: CollisionShapeComponent's can only work with uniform scale!");
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
    UpdateShapeLocalPose (_shape, _worldScale);
    UpdateShapeFilter (_shape);
}

const Memory::UniqueString Checkpoint::SIMULATION_STARTED {"PhysicsSimulationStarted"};
const Memory::UniqueString Checkpoint::SIMULATION_FINISHED {"PhysicsSimulationFinished"};

void AddToFixedUpdate (Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Celerity::TaskConstructor constructor =
        _pipelineBuilder.AddTask (Memory::UniqueString {"PhysicsSimulationExecutor"});
    constructor.SetExecutor<PhysicsSimulationExecutor> ();
}
} // namespace Emergence::Physics::Simulation
