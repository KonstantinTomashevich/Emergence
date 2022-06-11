#define _CRT_SECURE_NO_WARNINGS

#include <SyntaxSugar/MuteWarnings.hpp>

#include <fstream>
#include <thread>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/World.hpp>

#include <Gameplay/Assembly.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/HardcodedUnitTypes.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/UnitComponent.hpp>
#include <Gameplay/PhysicsConstant.hpp>

#include <Initialization/PhysicsInitialization.hpp>

#include <Input/Input.hpp>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Recording/StreamSerializer.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/Events.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>
#include <Render/Urho3DUpdate.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dVisualSync.hpp>

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#undef ERROR

using namespace Emergence::Memory::Literals;

class SceneSeeder final : public Emergence::Celerity::TaskExecutorBase<SceneSeeder>
{
public:
    SceneSeeder (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
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

SceneSeeder::SceneSeeder (Emergence::Celerity::TaskConstructor &_constructor) noexcept
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

void SceneSeeder::Execute ()
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto bodyCursor = insertRigidBody.Execute ();
    auto shapeCursor = insertCollisionShape.Execute ();

    auto cameraCursor = insertCamera.Execute ();
    auto lightCursor = insertLight.Execute ();
    auto modelCursor = insertStaticModel.Execute ();
    auto unitCursor = insertUnit.Execute ();

    const Emergence::Celerity::UniqueId cameraObjectId = world->GenerateUID ();

    auto *cameraTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (cameraObjectId);
    cameraTransform->SetVisualLocalTransform (
        {{4.0f, 7.0f, -1.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *camera = static_cast<CameraComponent *> (++cameraCursor);
    camera->objectId = cameraObjectId;
    camera->fieldOfViewRad = Emergence::Math::PI * 0.5f;

    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);
    renderScene->cameraObjectId = cameraObjectId;

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

    const Emergence::Celerity::UniqueId playerObjectId = world->GenerateUID ();

    auto *playerTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    playerTransform->SetObjectId (playerObjectId);
    playerTransform->SetLogicalLocalTransform (
        {{4.0f, 7.0f, 4.0f}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE}, true);

    auto *playerUnit = static_cast<UnitComponent *> (++unitCursor);
    playerUnit->objectId = playerObjectId;
    playerUnit->type = HardcodedUnitTypes::WARRIOR_CUBE;

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    const auto *physicsWorld = static_cast<const Emergence::Physics::PhysicsWorldSingleton *> (*physicsWorldCursor);

    for (std::size_t x = 0u; x < 9u; ++x)
    {
        for (std::size_t z = 0u; z < 9u; ++z)
        {
            const Emergence::Celerity::UniqueId objectId = world->GenerateUID ();

            auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
            transform->SetObjectId (objectId);
            transform->SetLogicalLocalTransform ({{static_cast<float> (x), 0.0f, static_cast<float> (z)},
                                                  Emergence::Math::Quaternion::IDENTITY,
                                                  Emergence::Math::Vector3f::ONE},
                                                 true);

            if (x == 0u || z == 0u || x == 8u || z == 8u)
            {
                auto *unit = static_cast<UnitComponent *> (++unitCursor);
                unit->objectId = objectId;
                unit->type = HardcodedUnitTypes::OBSTACLE;
            }
            else
            {
                auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
                body->objectId = objectId;
                body->type = Emergence::Physics::RigidBodyType::STATIC;

                auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
                shape->objectId = objectId;
                shape->shapeId = physicsWorld->GenerateShapeUID ();
                shape->materialId = PhysicsConstant::DEFAULT_MATERIAL_ID;

                shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                                   .boxHalfExtents = {0.5f, 0.01f, 0.5f}};
                shape->collisionGroup = PhysicsConstant::GROUND_COLLISION_GROUP;

                auto *model = static_cast<StaticModelComponent *> (++modelCursor);
                model->objectId = objectId;
                model->modelId = renderScene->GenerateModelUID ();

                model->modelName = "Models/FloorTile.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/FloorTileCenter.xml"_us);
                model->materialNames.EmplaceBack ("Materials/FloorTileBorder.xml"_us);
            }
        }
    }
}

static Emergence::Memory::Profiler::EventObserver StartRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

class GameApplication : public Urho3D::Application
{
    URHO3D_OBJECT (GameApplication, Application)

public:
    GameApplication (Urho3D::Context *_context);

    GameApplication (const GameApplication &_other) = delete;

    GameApplication (GameApplication &&_other) = delete;

    ~GameApplication () override = default;

    void Setup () override;

    void Start () override;

    void Stop () override;

    GameApplication &operator= (const GameApplication &_other) = delete;

    GameApplication &operator= (GameApplication &&_other) = delete;

private:
    void HandleUpdate (Urho3D::StringHash /*unused*/, Urho3D::VariantMap & /*unused*/) noexcept;

    void HandleKeyDown (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept;

    void HandleKeyUp (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept;

    std::ofstream memoryEventOutput;
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;
    Emergence::Memory::Profiler::EventObserver memoryEventObserver;

    Emergence::Celerity::InputAccumulator inputAccumulator;
    Emergence::Celerity::World world {"TestWorld"_us, {{1.0f / 60.0f}}};
};

GameApplication::GameApplication (Urho3D::Context *_context)
    : Application (_context),
      memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartRecording (memoryEventSerializer, memoryEventOutput))
{
    SubscribeToEvent (Urho3D::E_UPDATE, URHO3D_HANDLER (GameApplication, HandleUpdate));
    SubscribeToEvent (Urho3D::E_KEYDOWN, URHO3D_HANDLER (GameApplication, HandleKeyDown));
    SubscribeToEvent (Urho3D::E_KEYUP, URHO3D_HANDLER (GameApplication, HandleKeyUp));
}

void GameApplication::Setup ()
{
    Application::Setup ();
    engineParameters_[Urho3D::EP_FULL_SCREEN] = false;
    engineParameters_[Urho3D::EP_WINDOW_RESIZABLE] = true;
    engineParameters_[Urho3D::EP_RESOURCE_PATHS] = "Urho3DCoreAssets;GameAssets";
    engineParameters_[Urho3D::EP_RESOURCE_PREFIX_PATHS] = "..";
}

void GameApplication::Start ()
{
    Application::Start ();
    auto *input = GetSubsystem<Urho3D::Input> ();
    input->SetMouseVisible (true);
    input->SetMouseMode (Urho3D::MM_FREE);

    {
        Emergence::Celerity::EventRegistrar registrar {&world};
        RegisterGameplayEvents (registrar);
        RegisterRenderEvents (registrar);
        Emergence::Physics::RegisterEvents (registrar);
        Emergence::Transform::RegisterEvents (registrar);
    }

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};
    pipelineBuilder.Begin ("Initialization"_us, Emergence::Celerity::PipelineType::CUSTOM);
    PhysicsInitialization::AddToInitializationPipeline (pipelineBuilder);
    pipelineBuilder.AddTask ("SceneSeeder"_us).SetExecutor<SceneSeeder> ();
    Emergence::Celerity::Pipeline *initializer = pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Assembly::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    Emergence::Transform::VisualSync::AddToNormalUpdate (pipelineBuilder);
    Input::AddToNormalUpdate (&inputAccumulator, pipelineBuilder);
    Urho3DUpdate::AddToNormalUpdate (GetContext (), pipelineBuilder);
    // TODO: Calculate rational (for example, average parallel) amount of threads in Flow or TaskCollection?
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Assembly::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    Emergence::Physics::Simulation::AddToFixedUpdate (pipelineBuilder);
    Input::AddToFixedUpdate (pipelineBuilder);
    Mortality::AddToFixedUpdate (pipelineBuilder);
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    initializer->Execute ();
    world.RemovePipeline (initializer);
}

void GameApplication::Stop ()
{
    Application::Stop ();
}

void GameApplication::HandleUpdate (Urho3D::StringHash /*unused*/, Urho3D::VariantMap & /*unused*/) noexcept
{
    world.Update ();
    inputAccumulator.Clear ();

    while (const Emergence::Memory::Profiler::Event *event = memoryEventObserver.NextEvent ())
    {
        memoryEventSerializer.SerializeEvent (*event);
    }
}

void GameApplication::HandleKeyDown (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    if (_eventData[Urho3D::KeyDown::P_REPEAT].GetBool ())
    {
        return;
    }

    Emergence::Celerity::InputEvent event;
    event.type = Emergence::Celerity::InputType::KEYBOARD;
    event.keyboard = {
        static_cast<Emergence::Celerity::ScanCode> (_eventData[Urho3D::KeyDown::P_SCANCODE].GetInt ()),
        static_cast<Emergence::Celerity::KeyCode> (_eventData[Urho3D::KeyDown::P_KEY].GetInt ()),
        true,
        static_cast<Emergence::Celerity::QualifiersMask> (_eventData[Urho3D::KeyDown::P_QUALIFIERS].GetInt ()),
    };

    inputAccumulator.PostEvent (event);
}

void GameApplication::HandleKeyUp (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    Emergence::Celerity::InputEvent event;
    event.type = Emergence::Celerity::InputType::KEYBOARD;
    event.keyboard = {
        static_cast<Emergence::Celerity::ScanCode> (_eventData[Urho3D::KeyUp::P_SCANCODE].GetInt ()),
        static_cast<Emergence::Celerity::KeyCode> (_eventData[Urho3D::KeyUp::P_KEY].GetInt ()),
        false,
        static_cast<Emergence::Celerity::QualifiersMask> (_eventData[Urho3D::KeyUp::P_QUALIFIERS].GetInt ()),
    };

    inputAccumulator.PostEvent (event);
}

BEGIN_MUTING_WARNINGS
URHO3D_DEFINE_APPLICATION_MAIN (GameApplication)
END_MUTING_WARNINGS
