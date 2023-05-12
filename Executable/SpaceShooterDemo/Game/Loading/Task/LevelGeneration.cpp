#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/Physics3d/RigidBody3dComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <Container/Optional.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/PhysicsConstant.hpp>
#include <Gameplay/PlayerInfoSingleton.hpp>
#include <Gameplay/SpawnComponent.hpp>

#include <Loading/Model/Messages.hpp>
#include <Loading/Task/LevelGeneration.hpp>
#include <Loading/Task/PhysicsInitialization.hpp>

#include <Math/Constants.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>

namespace LevelGeneration
{
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LevelGenerationFinished"};

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

    void PlaceTransformWithAlignment (
        float _x,
        float _y,
        float _z,
        Emergence::Celerity::UniqueId _objectId,
        Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId = std::nullopt) noexcept;

    void PlaceDirectionalLight () noexcept;

    void PlaceSpawn (float _x,
                     float _y,
                     float _z,
                     Emergence::Memory::UniqueString _descriptorId,
                     Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId,
                     uint8_t _maxSpawnedObjects,
                     uint8_t _spawnCoolDownS) noexcept;

    void PlacePrototype (
        float _x,
        float _y,
        float _z,
        Emergence::Memory::UniqueString _descriptorId,
        Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId = std::nullopt) noexcept;

    Emergence::Celerity::ModifySequenceQuery modifyRequest;
    Emergence::Celerity::ModifySingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyRenderScene;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;
    Emergence::Celerity::FetchSingletonQuery fetchPlayerInfo;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertAlignment;
    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;
    Emergence::Celerity::InsertLongTermQuery insertSpawn;

    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertLight;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
    Emergence::Celerity::InsertLongTermQuery insertDamageDealer;
    Emergence::Celerity::InsertShortTermQuery insertResponse;
};

LevelGenerator::LevelGenerator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyRequest (MODIFY_SEQUENCE (LevelGenerationRequest)),
      fetchWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyRenderScene (MODIFY_SINGLETON (RenderSceneSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (Emergence::Celerity::PhysicsWorld3dSingleton)),
      fetchPlayerInfo (FETCH_SINGLETON (PlayerInfoSingleton)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform3dComponent)),
      insertAlignment (INSERT_LONG_TERM (AlignmentComponent)),
      insertRigidBody (INSERT_LONG_TERM (Emergence::Celerity::RigidBody3dComponent)),
      insertCollisionShape (INSERT_LONG_TERM (Emergence::Celerity::CollisionShape3dComponent)),
      insertSpawn (INSERT_LONG_TERM (SpawnComponent)),

      insertCamera (INSERT_LONG_TERM (CameraComponent)),
      insertLight (INSERT_LONG_TERM (LightComponent)),
      insertPrototype (INSERT_LONG_TERM (Emergence::Celerity::PrototypeComponent)),
      insertDamageDealer (INSERT_LONG_TERM (DamageDealerComponent)),
      insertResponse (INSERT_SHORT_TERM (LevelGenerationFinishedResponse))
{
    _constructor.DependOn (Emergence::Celerity::ResourceConfigLoading::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::ResourceObjectLoading::Checkpoint::FINISHED);
    _constructor.DependOn (PhysicsInitialization::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void LevelGenerator::Execute ()
{
    auto requestCursor = modifyRequest.Execute ();
    if (!*requestCursor)
    {
        return;
    }

    ~requestCursor;
    auto playerInfoCursor = fetchPlayerInfo.Execute ();
    const auto *playerInfo = static_cast<const PlayerInfoSingleton *> (*playerInfoCursor);

    PlaceFloor (30, 20);
    PlaceKillZ (60.0f, 40.0f, -1.0f);
    PlaceCamera ();
    PlaceDirectionalLight ();

    bool placeRed = true;
    for (std::int32_t x = -30; x < 30; x += 6)
    {
        for (std::int32_t z = -20; z < 20; z += 6)
        {
            PlaceSpawn (static_cast<float> (x) + 0.5f, 0.0f, static_cast<float> (z) + 0.5f,
                        placeRed ? "Structure/ObstacleRed"_us : "Structure/ObstacleYellow"_us, std::nullopt, 1u, 15u);
            placeRed = !placeRed;
        }
    }

    PlaceSpawn (-2.0f, 2.5f, 0.0f, "Unit/Fighter"_us, playerInfo->localPlayerUid, 1u, 2u);
    const Emergence::Celerity::UniqueId aiPlayerId = playerInfo->GeneratePlayerId ();

    for (std::int32_t x = -27; x < 30; x += 18)
    {
        for (std::int32_t z = -17; z < 20; z += 15)
        {
            PlaceSpawn (static_cast<float> (x) + 0.5f, 2.5f, static_cast<float> (z) + 0.5f, "Unit/Fighter"_us,
                        aiPlayerId, 2u, 5u);
        }
    }

    auto responseCursor = insertResponse.Execute ();
    ++responseCursor;
}

void LevelGenerator::PlaceFloor (std::int32_t _halfWidth, std::int32_t _halfHeight) noexcept
{
    for (std::int32_t x = -_halfWidth; x < _halfWidth; ++x)
    {
        for (std::int32_t z = -_halfHeight; z < _halfHeight; ++z)
        {
            PlacePrototype (static_cast<float> (x) + 0.5f, 0.0f, static_cast<float> (z) + 0.5f,
                            "Structure/FloorTile"_us);
        }
    }
}

void LevelGenerator::PlaceKillZ (float _halfWidth, float _halfHeight, float _z) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Celerity::PhysicsWorld3dSingleton *> (*physicsWorldCursor);

    const Emergence::Celerity::UniqueId killZObjectId = world->GenerateId ();
    auto transformCursor = insertTransform.Execute ();
    auto bodyCursor = insertRigidBody.Execute ();
    auto shapeCursor = insertCollisionShape.Execute ();
    auto damageDealerCursor = insertDamageDealer.Execute ();

    auto *transform = static_cast<Emergence::Celerity::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (killZObjectId);
    transform->SetLogicalLocalTransform (
        {{0.0f, _z, 0.0f}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE}, true);

    auto *body = static_cast<Emergence::Celerity::RigidBody3dComponent *> (++bodyCursor);
    body->objectId = killZObjectId;
    body->type = Emergence::Celerity::RigidBody3dType::STATIC;

    auto *shape = static_cast<Emergence::Celerity::CollisionShape3dComponent *> (++shapeCursor);
    shape->objectId = killZObjectId;
    shape->shapeId = physicsWorld->GenerateShapeId ();
    shape->materialId = "Default"_us;

    shape->geometry = {.type = Emergence::Celerity::CollisionGeometry3dType::BOX,
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

    const Emergence::Celerity::UniqueId cameraObjectId = world->GenerateId ();

    auto *cameraTransform = static_cast<Emergence::Celerity::Transform3dComponent *> (++transformCursor);
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

    const Emergence::Celerity::UniqueId lightObjectId = world->GenerateId ();

    auto *lightTransform = static_cast<Emergence::Celerity::Transform3dComponent *> (++transformCursor);
    lightTransform->SetObjectId (lightObjectId);
    lightTransform->SetVisualLocalTransform (
        {{0.0f, 5.0f, 0.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *light = static_cast<LightComponent *> (++lightCursor);
    light->objectId = lightObjectId;
    light->lightId = renderScene->GenerateLightId ();
    light->type = LightType::DIRECTIONAL;
    light->color = {1.0f, 1.0f, 1.0f, 1.0f};
}

void LevelGenerator::PlaceTransformWithAlignment (
    float _x,
    float _y,
    float _z,
    Emergence::Celerity::UniqueId _objectId,
    Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId) noexcept
{
    auto transformCursor = insertTransform.Execute ();
    auto *transform = static_cast<Emergence::Celerity::Transform3dComponent *> (++transformCursor);
    transform->SetObjectId (_objectId);

    transform->SetLogicalLocalTransform (
        {{_x, _y, _z}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE}, true);

    if (_playerId)
    {
        auto alignmentCursor = insertAlignment.Execute ();
        auto *alignment = static_cast<AlignmentComponent *> (++alignmentCursor);
        alignment->objectId = _objectId;
        alignment->playerId = _playerId.value ();
    }
}

void LevelGenerator::PlacePrototype (float _x,
                                     float _y,
                                     float _z,
                                     Emergence::Memory::UniqueString _descriptorId,
                                     Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    const Emergence::Celerity::UniqueId objectId = world->GenerateId ();
    PlaceTransformWithAlignment (_x, _y, _z, objectId, _playerId);

    auto prototypeCursor = insertPrototype.Execute ();
    auto *prototype = static_cast<Emergence::Celerity::PrototypeComponent *> (++prototypeCursor);
    prototype->objectId = objectId;
    prototype->descriptorId = _descriptorId;
    prototype->requestImmediateFixedAssembly = false;
    prototype->requestImmediateNormalAssembly = false;
}

void LevelGenerator::PlaceSpawn (float _x,
                                 float _y,
                                 float _z,
                                 Emergence::Memory::UniqueString _descriptorId,
                                 Emergence::Container::Optional<Emergence::Celerity::UniqueId> _playerId,
                                 uint8_t _maxSpawnedObjects,
                                 uint8_t _spawnCoolDownS) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    const Emergence::Celerity::UniqueId objectId = world->GenerateId ();
    PlaceTransformWithAlignment (_x, _y, _z, objectId, _playerId);

    auto spawnCursor = insertSpawn.Execute ();
    auto *spawn = static_cast<SpawnComponent *> (++spawnCursor);
    spawn->objectId = objectId;
    spawn->objectToSpawnId = _descriptorId;
    spawn->maxSpawnedObjects = _maxSpawnedObjects;
    spawn->spawnCoolDownNs = static_cast<uint64_t> (_spawnCoolDownS) * 1000000000u;
}

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("LevelGeneration");
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("LevelGenerator"_us).SetExecutor<LevelGenerator> ();
}
} // namespace LevelGeneration
