#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Assembly.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/HardcodedPrototypes.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/PhysicsConstant.hpp>
#include <Gameplay/PrototypeComponent.hpp>
#include <Gameplay/UnitComponent.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Math/Constants.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

#include <Shared/Checkpoint.hpp>

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

    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Emergence::Celerity::FetchValueQuery fetchPrototypeById;

    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;
    Emergence::Celerity::InsertLongTermQuery insertUnit;
    Emergence::Celerity::InsertLongTermQuery insertInputListener;
    Emergence::Celerity::InsertLongTermQuery insertMovement;
};

FixedAssembler::FixedAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPrototypeAddedFixedEvents (_constructor.MFetchSequence (PrototypeComponentAddedFixedEvent)),
      fetchPrototypeAddedCustomToFixedEvents (_constructor.MFetchSequence (PrototypeComponentAddedCustomToFixedEvent)),

      fetchPhysicsWorld (_constructor.MFetchSingleton (Emergence::Physics::PhysicsWorldSingleton)),
      fetchPrototypeById (_constructor.MFetchValue1F (PrototypeComponent, objectId)),

      insertRigidBody (_constructor.MInsertLongTerm (Emergence::Physics::RigidBodyComponent)),
      insertCollisionShape (_constructor.MInsertLongTerm (Emergence::Physics::CollisionShapeComponent)),
      insertUnit (_constructor.MInsertLongTerm (UnitComponent)),
      insertInputListener (_constructor.MInsertLongTerm (InputListenerComponent)),
      insertMovement (_constructor.MInsertLongTerm (MovementComponent))
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.MakeDependencyOf (Emergence::Physics::Simulation::Checkpoint::SIMULATION_STARTED);
}

void FixedAssembler::Execute ()
{
    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    auto assembly = [this, physicsWorld] (Emergence::Celerity::UniqueId _objectId)
    {
        auto prototypeCursor = fetchPrototypeById.Execute (&_objectId);
        if (const auto *prototype = static_cast<const PrototypeComponent *> (*prototypeCursor))
        {
            auto unitCursor = insertUnit.Execute ();
            auto bodyCursor = insertRigidBody.Execute ();
            auto shapeCursor = insertCollisionShape.Execute ();

            if (prototype->prototype == HardcodedPrototypes::WARRIOR_CUBE)
            {
                auto inputListenerCursor = insertInputListener.Execute ();
                auto movementCursor = insertMovement.Execute ();

                auto *unit = static_cast<UnitComponent *> (++unitCursor);
                unit->objectId = _objectId;
                unit->canBeControlledByPlayer = true;

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
            }
            else if (prototype->prototype == HardcodedPrototypes::OBSTACLE)
            {
                auto *unit = static_cast<UnitComponent *> (++unitCursor);
                unit->objectId = _objectId;
                unit->canBeControlledByPlayer = false;

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
}

class NormalAssembler final : public Emergence::Celerity::TaskExecutorBase<NormalAssembler>
{
public:
    NormalAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedFixedToNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchPrototypeAddedCustomToNormalEvents;

    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;
    Emergence::Celerity::FetchValueQuery fetchPrototypeById;
    Emergence::Celerity::InsertLongTermQuery insertStaticModel;
};

NormalAssembler::NormalAssembler (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPrototypeAddedFixedToNormalEvents (_constructor.MFetchSequence (PrototypeComponentAddedFixedToNormalEvent)),
      fetchPrototypeAddedCustomToNormalEvents (
          _constructor.MFetchSequence (PrototypeComponentAddedCustomToNormalEvent)),

      fetchRenderScene (_constructor.MFetchSingleton (RenderSceneSingleton)),
      fetchPrototypeById (_constructor.MFetchValue1F (PrototypeComponent, objectId)),
      insertStaticModel (_constructor.MInsertLongTerm (StaticModelComponent))
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
