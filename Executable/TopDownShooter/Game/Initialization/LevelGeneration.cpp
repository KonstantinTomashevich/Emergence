#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/HardcodedUnitTypes.hpp>
#include <Gameplay/PhysicsConstant.hpp>
#include <Gameplay/UnitComponent.hpp>

#include <Initialization/LevelGeneration.hpp>
#include <Initialization/PhysicsInitialization.hpp>

#include <Math/Constants.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

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

    void PlaceCamera () noexcept;

    void PlaceDirectionalLight () noexcept;

    void PlaceUnit (std::int32_t _x,
                    std::uint32_t _y,
                    std::int32_t _z,
                    Emergence::Memory::UniqueString _type,
                    bool _canBeControlledByPlayer) noexcept;

    Emergence::Celerity::ModifySingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyRenderScene;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;

    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertLight;
    Emergence::Celerity::InsertLongTermQuery insertStaticModel;
    Emergence::Celerity::InsertLongTermQuery insertUnit;
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
      insertStaticModel (_constructor.MInsertLongTerm (StaticModelComponent)),
      insertUnit (_constructor.MInsertLongTerm (UnitComponent))
{
    _constructor.DependOn (PhysicsInitialization::Checkpoint::PHYSICS_INITIALIZED);
}

void LevelGenerator::Execute ()
{
    PlaceFloor (30, 20);
    PlaceCamera ();
    PlaceDirectionalLight ();

    for (std::int32_t x = -30; x < 30; x += 4)
    {
        for (std::int32_t z = -20; z < 20; z += 4)
        {
            PlaceUnit (x, 0, z, HardcodedUnitTypes::OBSTACLE, false);
        }
    }

    PlaceUnit (0, 1, 0, HardcodedUnitTypes::WARRIOR_CUBE, true);
}

void LevelGenerator::PlaceFloor (std::int32_t _halfWidth, std::int32_t _halfHeight) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto modelCursor = insertStaticModel.Execute ();

    for (std::int32_t x = -_halfWidth; x < _halfWidth; ++x)
    {
        for (std::int32_t z = -_halfHeight; z < _halfWidth; ++z)
        {
            const Emergence::Celerity::UniqueId objectId = world->GenerateUID ();

            auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
            transform->SetObjectId (objectId);
            transform->SetLogicalLocalTransform ({{static_cast<float> (x) + 0.5f, 0.0f, static_cast<float> (z) + 0.5f},
                                                  Emergence::Math::Quaternion::IDENTITY,
                                                  Emergence::Math::Vector3f::ONE},
                                                 true);

            auto *model = static_cast<StaticModelComponent *> (++modelCursor);
            model->objectId = objectId;
            model->modelId = renderScene->GenerateModelUID ();

            model->modelName = "Models/FloorTile.mdl"_us;
            model->materialNames.EmplaceBack ("Materials/FloorTileCenter.xml"_us);
            model->materialNames.EmplaceBack ("Materials/FloorTileBorder.xml"_us);
        }
    }

    const Emergence::Celerity::UniqueId groundShapeObjectId = world->GenerateUID ();
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

void LevelGenerator::PlaceUnit (std::int32_t _x,
                                std::uint32_t _y,
                                std::int32_t _z,
                                Emergence::Memory::UniqueString _type,
                                bool _canBeControlledByPlayer) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto unitCursor = insertUnit.Execute ();

    const Emergence::Celerity::UniqueId objectId = world->GenerateUID ();
    auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (objectId);
    transform->SetLogicalLocalTransform (
        {{static_cast<float> (_x) + 0.5f, static_cast<float> (_y), static_cast<float> (_z + 0.5f)},
         Emergence::Math::Quaternion::IDENTITY,
         Emergence::Math::Vector3f::ONE},
        true);

    auto *unit = static_cast<UnitComponent *> (++unitCursor);
    unit->objectId = objectId;
    unit->type = _type;
    unit->canBeControlledByPlayer = _canBeControlledByPlayer;
}

void AddToInitializationPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("LevelGenerator"_us).SetExecutor<LevelGenerator> ();
}
} // namespace LevelGeneration
