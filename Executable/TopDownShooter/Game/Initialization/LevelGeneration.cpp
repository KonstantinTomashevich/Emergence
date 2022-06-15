#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/HardcodedPrototypes.hpp>
#include <Gameplay/PhysicsConstant.hpp>
#include <Gameplay/PrototypeComponent.hpp>

#include <Initialization/LevelGeneration.hpp>
#include <Initialization/PhysicsInitialization.hpp>

#include <Math/Constants.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>

#include <Transform/Transform3dComponent.hpp>

namespace LevelGeneration
{
using namespace Emergence::Memory::Literals;

class LevelGenerator final : public Emergence::Celerity::TaskExecutorBase<LevelGenerator>
{
public:
    LevelGenerator (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void PlaceFloor (std::int32_t _halfWidth, std::int32_t _halfHeight) noexcept;

    void PlaceKillZ (float _halfWidth, float _halfHeight, float _z) noexcept;

    void PlaceCamera () noexcept;

    void PlaceDirectionalLight () noexcept;

    void PlacePrototype (float _x, float _y, float _z, Emergence::Memory::UniqueString _prototype) noexcept;

    Emergence::Celerity::ModifySingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyRenderScene;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;

    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertLight;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
    Emergence::Celerity::InsertLongTermQuery insertDamageDealer;
};

LevelGenerator::LevelGenerator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchWorld (_constructor.MModifySingleton (Emergence::Celerity::WorldSingleton)),
      modifyRenderScene (_constructor.MModifySingleton (RenderSceneSingleton)),
      fetchPhysicsWorld (_constructor.MFetchSingleton (Emergence::Physics::PhysicsWorldSingleton)),

      insertTransform (_constructor.MInsertLongTerm (Emergence::Transform::Transform3dComponent)),
      insertRigidBody (_constructor.MInsertLongTerm (Emergence::Physics::RigidBodyComponent)),
      insertCollisionShape (_constructor.MInsertLongTerm (Emergence::Physics::CollisionShapeComponent)),

      insertCamera (_constructor.MInsertLongTerm (CameraComponent)),
      insertLight (_constructor.MInsertLongTerm (LightComponent)),
      insertPrototype (_constructor.MInsertLongTerm (PrototypeComponent)),
      insertDamageDealer (_constructor.MInsertLongTerm (DamageDealerComponent))
{
    _constructor.DependOn (PhysicsInitialization::Checkpoint::PHYSICS_INITIALIZED);
}

void LevelGenerator::Execute ()
{
    PlaceFloor (30, 20);
    PlaceKillZ (60.0f, 40.0f, -1.0f);
    PlaceCamera ();
    PlaceDirectionalLight ();

    for (std::int32_t x = -30; x < 30; x += 6)
    {
        for (std::int32_t z = -20; z < 20; z += 6)
        {
            PlacePrototype (static_cast<float> (x) + 0.5f, 0.0f, static_cast<float> (z) + 0.5f,
                            HardcodedPrototypes::OBSTACLE);
        }
    }

    PlacePrototype (-2.0f, 0.5f, 0.0f, HardcodedPrototypes::WARRIOR_CUBE);
}

void LevelGenerator::PlaceFloor (std::int32_t _halfWidth, std::int32_t _halfHeight) noexcept
{
    for (std::int32_t x = -_halfWidth; x < _halfWidth; ++x)
    {
        for (std::int32_t z = -_halfHeight; z < _halfHeight; ++z)
        {
            PlacePrototype (static_cast<float> (x) + 0.5f, 0.0f, static_cast<float> (z) + 0.5f,
                            HardcodedPrototypes::FLOOR_TILE);
        }
    }

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    const Emergence::Celerity::UniqueId groundShapeObjectId = world->GenerateUID ();
    auto transformCursor = insertTransform.Execute ();
    auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (groundShapeObjectId);

    auto bodyCursor = insertRigidBody.Execute ();
    auto shapeCursor = insertCollisionShape.Execute ();

    auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
    body->objectId = groundShapeObjectId;
    body->type = Emergence::Physics::RigidBodyType::STATIC;

    auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
    shape->objectId = groundShapeObjectId;
    shape->shapeId = physicsWorld->GenerateShapeUID ();
    shape->materialId = PhysicsConstant::DEFAULT_MATERIAL_ID;

    shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                       .boxHalfExtents = {static_cast<float> (_halfWidth), 0.5f, static_cast<float> (_halfHeight)}};
    shape->translation.y = -0.5f;
    shape->collisionGroup = PhysicsConstant::GROUND_COLLISION_GROUP;
}

void LevelGenerator::PlaceKillZ (float _halfWidth, float _halfHeight, float _z) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    const Emergence::Celerity::UniqueId killZObjectId = world->GenerateUID ();
    auto transformCursor = insertTransform.Execute ();
    auto bodyCursor = insertRigidBody.Execute ();
    auto shapeCursor = insertCollisionShape.Execute ();
    auto damageDealerCursor = insertDamageDealer.Execute ();

    auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (killZObjectId);
    transform->SetLogicalLocalTransform (
        {{0.0f, _z, 0.0f}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE});

    auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
    body->objectId = killZObjectId;
    body->type = Emergence::Physics::RigidBodyType::STATIC;

    auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
    shape->objectId = killZObjectId;
    shape->shapeId = physicsWorld->GenerateShapeUID ();
    shape->materialId = PhysicsConstant::DEFAULT_MATERIAL_ID;

    shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                       .boxHalfExtents = {_halfWidth, physicsWorld->toleranceSpeed, _halfHeight}};

    shape->translation.y = -physicsWorld->toleranceSpeed;
    shape->collisionGroup = PhysicsConstant::OBSTACLE_COLLISION_GROUP;
    shape->trigger = true;

    auto *damageDealer = static_cast<DamageDealerComponent *> (++damageDealerCursor);
    damageDealer->objectId = killZObjectId;
    damageDealer->damage = 1000000.0f;
    damageDealer->multiUse = true;
}

void LevelGenerator::PlaceCamera () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto cameraCursor = insertCamera.Execute ();

    const Emergence::Celerity::UniqueId cameraObjectId = world->GenerateUID ();

    auto *cameraTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (cameraObjectId);
    cameraTransform->SetVisualLocalTransform (
        {{0.0f, 7.0f, -1.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *camera = static_cast<CameraComponent *> (++cameraCursor);
    camera->objectId = cameraObjectId;
    camera->fieldOfViewRad = Emergence::Math::PI * 0.5f;

    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);
    renderScene->cameraObjectId = cameraObjectId;
}

void LevelGenerator::PlaceDirectionalLight () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);

    auto transformCursor = insertTransform.Execute ();
    auto lightCursor = insertLight.Execute ();

    const Emergence::Celerity::UniqueId lightObjectId = world->GenerateUID ();

    auto *lightTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    lightTransform->SetObjectId (lightObjectId);
    lightTransform->SetVisualLocalTransform (
        {{0.0f, 5.0f, 0.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *light = static_cast<LightComponent *> (++lightCursor);
    light->objectId = lightObjectId;
    light->lightId = renderScene->GenerateLightUID ();
    light->type = LightType::DIRECTIONAL;
    light->color = {1.0f, 1.0f, 1.0f, 1.0f};
}

void LevelGenerator::PlacePrototype (float _x, float _y, float _z, Emergence::Memory::UniqueString _prototype) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto prototypeCursor = insertPrototype.Execute ();

    const Emergence::Celerity::UniqueId objectId = world->GenerateUID ();
    auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (objectId);

    transform->SetLogicalLocalTransform (
        {{_x, _y, _z}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE}, true);

    auto *unit = static_cast<PrototypeComponent *> (++prototypeCursor);
    unit->objectId = objectId;
    unit->prototype = _prototype;
}

void AddToInitializationPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("LevelGenerator"_us).SetExecutor<LevelGenerator> ();
}
} // namespace LevelGeneration
