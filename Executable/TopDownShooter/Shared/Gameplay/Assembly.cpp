#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/Assembly.hpp>
#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/HardcodedPrototypes.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/PhysicsConstant.hpp>
#include <Gameplay/PrototypeComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Math/Constants.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Assembly
{
// TODO: Normally assembly pattern uses special configs to assemble entities,
//       but in demo we just hardcode this thing for simplicity.

using namespace Emergence::Memory::Literals;

class FixedAssembler final : public Emergence::Celerity::TaskExecutorBase<FixedAssembler>
{
public:
    FixedAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedFixedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedCustomToFixedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;

    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Emergence::Celerity::FetchValueQuery fetchPrototypeById;
    Emergence::Celerity::RemoveValueQuery removePrototypeById;
    Emergence::Celerity::RemoveValueQuery removeAlignmentById;

    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;
    Emergence::Celerity::InsertLongTermQuery insertMortal;
    Emergence::Celerity::InsertLongTermQuery insertControllable;
    Emergence::Celerity::InsertLongTermQuery insertInputListener;
    Emergence::Celerity::InsertLongTermQuery insertMovement;
    Emergence::Celerity::InsertLongTermQuery insertShooter;
    Emergence::Celerity::InsertLongTermQuery insertDamageDealer;

    Emergence::Celerity::InsertShortTermQuery insertPrototypeAssembledEvent;
};

FixedAssembler::FixedAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPrototypeAddedFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedEvent)),
      fetchPrototypeAddedCustomToFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToFixedEvent)),
      fetchTransformRemovedEvents (FETCH_SEQUENCE (Emergence::Transform::Transform3dComponentRemovedFixedEvent)),

      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (Emergence::Physics::PhysicsWorldSingleton)),
      fetchPrototypeById (FETCH_VALUE_1F (PrototypeComponent, objectId)),
      removePrototypeById (REMOVE_VALUE_1F (PrototypeComponent, objectId)),
      removeAlignmentById (REMOVE_VALUE_1F (AlignmentComponent, objectId)),

      fetchTransformById (FETCH_VALUE_1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertTransform (INSERT_LONG_TERM (Emergence::Transform::Transform3dComponent)),
      insertRigidBody (INSERT_LONG_TERM (Emergence::Physics::RigidBodyComponent)),
      insertCollisionShape (INSERT_LONG_TERM (Emergence::Physics::CollisionShapeComponent)),
      insertMortal (INSERT_LONG_TERM (MortalComponent)),
      insertControllable (INSERT_LONG_TERM (ControllableComponent)),
      insertInputListener (INSERT_LONG_TERM (InputListenerComponent)),
      insertMovement (INSERT_LONG_TERM (MovementComponent)),
      insertShooter (INSERT_LONG_TERM (ShooterComponent)),
      insertDamageDealer (INSERT_LONG_TERM (DamageDealerComponent)),

      insertPrototypeAssembledEvent (INSERT_SHORT_TERM (PrototypeAssembledFixedEvent))
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.MakeDependencyOf (Emergence::Physics::Simulation::Checkpoint::SIMULATION_STARTED);
}

void FixedAssembler::Execute ()
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    auto assembly = [this, world, physicsWorld] (Emergence::Celerity::UniqueId _objectId)
    {
        auto prototypeCursor = fetchPrototypeById.Execute (&_objectId);
        if (const auto *prototype = static_cast<const PrototypeComponent *> (*prototypeCursor))
        {
            auto mortalCursor = insertMortal.Execute ();
            auto bodyCursor = insertRigidBody.Execute ();
            auto shapeCursor = insertCollisionShape.Execute ();

            if (prototype->prototype == HardcodedPrototypes::WARRIOR_CUBE)
            {
                auto controllableCursor = insertControllable.Execute ();
                auto transformCursor = insertTransform.Execute ();
                auto inputListenerCursor = insertInputListener.Execute ();
                auto movementCursor = insertMovement.Execute ();
                auto shooterCursor = insertShooter.Execute ();

                auto *mortal = static_cast<MortalComponent *> (++mortalCursor);
                mortal->objectId = _objectId;

                auto *controllable = static_cast<ControllableComponent *> (++controllableCursor);
                controllable->objectId = _objectId;

                auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
                body->objectId = _objectId;
                body->angularDamping = 0.99f;
                body->type = Emergence::Physics::RigidBodyType::DYNAMIC;
                body->manipulatedOutsideOfSimulation = true;

                body->lockFlags = Emergence::Physics::RigidBodyComponent::LOCK_ANGULAR_X |
                                  Emergence::Physics::RigidBodyComponent::LOCK_ANGULAR_Z;

                // Sphere for movement

                auto *movementShape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
                movementShape->objectId = _objectId;
                movementShape->shapeId = physicsWorld->GenerateShapeUID ();
                movementShape->materialId = "Default"_us;

                movementShape->geometry = {.type = Emergence::Physics::CollisionGeometryType::SPHERE,
                                           .sphereRadius = 0.5f};
                movementShape->collisionGroup = PhysicsConstant::WARRIOR_COLLISION_GROUP;

                // Hitbox for bullets.

                auto *hitBoxShape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
                hitBoxShape->objectId = _objectId;
                hitBoxShape->shapeId = physicsWorld->GenerateShapeUID ();
                hitBoxShape->materialId = "Default"_us;

                hitBoxShape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                                         .boxHalfExtents = {0.5f, 0.5f, 0.5f}};
                hitBoxShape->collisionGroup = PhysicsConstant::HIT_BOX_COLLISION_GROUP;
                hitBoxShape->trigger = true;

                auto *inputListener = static_cast<InputListenerComponent *> (++inputListenerCursor);
                inputListener->objectId = _objectId;

                auto *movement = static_cast<MovementComponent *> (++movementCursor);
                movement->objectId = _objectId;

                movement->maxLinearSpeed = 5.0f;
                movement->linearAcceleration = Emergence::Math::Vector3f::FORWARD * 5.0f;

                movement->maxAngularSpeed = Emergence::Math::PI;
                movement->angularAcceleration = {0.0f, Emergence::Math::PI, 0.0f};

                movement->linearVelocityMask = 0b00000101u;  // Only forward and to the sides.
                movement->angularVelocityMask = 0b00000010u; // Only around Y axis.

                auto *shooter = static_cast<ShooterComponent *> (++shooterCursor);
                shooter->objectId = _objectId;
                shooter->coolDownNs = 500000000u; // 0.5s
                shooter->bulletPrototype = HardcodedPrototypes::BULLET;

                auto *shootingPointTransform =
                    static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);

                shootingPointTransform->SetObjectId (world->GenerateUID ());
                shootingPointTransform->SetParentObjectId (_objectId);

                shootingPointTransform->SetLogicalLocalTransform (
                    {{0.0f, 0.0f, 1.0f}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE});

                shooter->shootingPointObjectId = shootingPointTransform->GetObjectId ();
            }
            else if (prototype->prototype == HardcodedPrototypes::OBSTACLE)
            {
                auto *mortal = static_cast<MortalComponent *> (++mortalCursor);
                mortal->objectId = _objectId;

                auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
                body->objectId = _objectId;
                body->type = Emergence::Physics::RigidBodyType::STATIC;

                auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
                shape->objectId = _objectId;
                shape->shapeId = physicsWorld->GenerateShapeUID ();
                shape->materialId = "Default"_us;

                shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                                   .boxHalfExtents = {0.5f, 1.5f, 0.5f}};
                shape->translation.y = 1.5f;
                shape->collisionGroup = PhysicsConstant::OBSTACLE_COLLISION_GROUP;
            }
            else if (prototype->prototype == HardcodedPrototypes::BULLET)
            {
                auto damageDealerCursor = insertDamageDealer.Execute ();

                Emergence::Math::Quaternion bulletRotation = Emergence::Math::Quaternion::IDENTITY;
                auto transformCursor = fetchTransformById.Execute (&prototype->objectId);

                if (const auto *transform =
                        static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor))
                {
                    bulletRotation = transform->GetLogicalWorldTransform (transformWorldAccessor).rotation;
                }

                auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
                body->objectId = _objectId;
                body->type = Emergence::Physics::RigidBodyType::DYNAMIC;
                body->linearVelocity = Emergence::Math::Rotate ({0.0f, 0.0f, 50.0f}, bulletRotation);

                auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
                shape->objectId = _objectId;
                shape->shapeId = physicsWorld->GenerateShapeUID ();
                shape->materialId = "Default"_us;
                shape->sendContactEvents = true;

                shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::SPHERE, .sphereRadius = 0.2f};
                shape->collisionGroup = PhysicsConstant::BULLET_COLLISION_GROUP;

                auto *damageDealer = static_cast<DamageDealerComponent *> (++damageDealerCursor);
                damageDealer->objectId = _objectId;
                damageDealer->damage = 1.0f;
                damageDealer->multiUse = false;
            }

            auto eventCursor = insertPrototypeAssembledEvent.Execute ();
            auto *event = static_cast<PrototypeAssembledFixedEvent *> (++eventCursor);
            event->objectId = prototype->objectId;
        }
    };

    for (auto eventCursor = fetchPrototypeAddedFixedEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        assembly (event->objectId);
    }

    for (auto eventCursor = fetchPrototypeAddedCustomToFixedEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        assembly (event->objectId);
    }

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto prototypeCursor = removePrototypeById.Execute (&event->objectId);
        if (prototypeCursor.ReadConst ())
        {
            ~prototypeCursor;
        }

        // TODO: Currently we are clearing non-mechanic-specific components here.

        auto alignmentCursor = removeAlignmentById.Execute (&event->objectId);
        if (alignmentCursor.ReadConst ())
        {
            ~alignmentCursor;
        }
    }
}

class NormalAssembler final : public Emergence::Celerity::TaskExecutorBase<NormalAssembler>
{
public:
    NormalAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedFixedToNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedCustomToNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;

    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;
    Emergence::Celerity::FetchValueQuery fetchPrototypeById;
    Emergence::Celerity::RemoveValueQuery removePrototypeById;
    Emergence::Celerity::InsertLongTermQuery insertStaticModel;

    Emergence::Celerity::InsertShortTermQuery insertPrototypeAssembledEvent;
};

NormalAssembler::NormalAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPrototypeAddedFixedToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedToNormalEvent)),
      fetchPrototypeAddedCustomToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToNormalEvent)),
      fetchTransformRemovedEvents (FETCH_SEQUENCE (Emergence::Transform::Transform3dComponentRemovedNormalEvent)),

      fetchRenderScene (FETCH_SINGLETON (RenderSceneSingleton)),
      fetchPrototypeById (FETCH_VALUE_1F (PrototypeComponent, objectId)),
      removePrototypeById (REMOVE_VALUE_1F (PrototypeComponent, objectId)),
      insertStaticModel (INSERT_LONG_TERM (StaticModelComponent)),

      insertPrototypeAssembledEvent (INSERT_SHORT_TERM (PrototypeAssembledNormalEvent))
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_UPDATE_STARTED);
}

void NormalAssembler::Execute ()
{
    auto renderSceneCursor = fetchRenderScene.Execute ();
    const auto *renderScene = static_cast<const RenderSceneSingleton *> (*renderSceneCursor);

    auto assembly = [this, renderScene] (Emergence::Celerity::UniqueId _objectId)
    {
        auto prototypeCursor = fetchPrototypeById.Execute (&_objectId);
        if (const auto *prototype = static_cast<const PrototypeComponent *> (*prototypeCursor))
        {
            auto modelCursor = insertStaticModel.Execute ();
            auto *model = static_cast<StaticModelComponent *> (++modelCursor);
            model->objectId = _objectId;
            model->modelId = renderScene->GenerateModelUID ();

            if (prototype->prototype == HardcodedPrototypes::WARRIOR_CUBE)
            {
                model->modelName = "Models/Player.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/Player.xml"_us);
            }
            else if (prototype->prototype == HardcodedPrototypes::OBSTACLE)
            {
                model->modelName = "Models/Wall.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/WallTileBorder.xml"_us);
                model->materialNames.EmplaceBack ("Materials/WallTileCenter.xml"_us);
            }
            else if (prototype->prototype == HardcodedPrototypes::FLOOR_TILE)
            {
                model->modelName = "Models/FloorTile.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/FloorTileCenter.xml"_us);
                model->materialNames.EmplaceBack ("Materials/FloorTileBorder.xml"_us);
            }
            else if (prototype->prototype == HardcodedPrototypes::BULLET)
            {
                model->modelName = "Models/Bullet.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/Bullet.xml"_us);
            }

            auto eventCursor = insertPrototypeAssembledEvent.Execute ();
            auto *event = static_cast<PrototypeAssembledNormalEvent *> (++eventCursor);
            event->objectId = prototype->objectId;
        }
    };

    for (auto eventCursor = fetchPrototypeAddedFixedToNormalEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        assembly (event->objectId);
    }

    for (auto eventCursor = fetchPrototypeAddedCustomToNormalEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedCustomToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        assembly (event->objectId);
    }

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto prototypeCursor = removePrototypeById.Execute (&event->objectId);
        if (prototypeCursor.ReadConst ())
        {
            ~prototypeCursor;
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::FixedUpdate"_us).SetExecutor<FixedAssembler> ();
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::NormalUpdate"_us).SetExecutor<NormalAssembler> ();
}
} // namespace Assembly
