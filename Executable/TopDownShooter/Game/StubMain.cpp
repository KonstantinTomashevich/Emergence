#define _CRT_SECURE_NO_WARNINGS

#include <SyntaxSugar/MuteWarnings.hpp>

#include <fstream>
#include <thread>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/World.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/Mortality.hpp>

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

static constexpr Emergence::Celerity::UniqueId CAMERA_OBJECT_ID = 0u;
static constexpr Emergence::Celerity::UniqueId LIGHT_OBJECT_ID = 1u;
static constexpr Emergence::Celerity::UniqueId PLAYER_OBJECT_ID = 2u;
static constexpr Emergence::Celerity::UniqueId OTHER_OBJECTS_START_ID = 10u;

class SceneSeeder final : public Emergence::Celerity::TaskExecutorBase<SceneSeeder>
{
public:
    SceneSeeder (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::ModifySingletonQuery modifyRenderScene;
    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertDynamicsMaterial;
    Emergence::Celerity::InsertLongTermQuery insertRigidBody;
    Emergence::Celerity::InsertLongTermQuery insertCollisionShape;

    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertLight;
    Emergence::Celerity::InsertLongTermQuery insertStaticModel;
};

SceneSeeder::SceneSeeder (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyRenderScene (_constructor.MModifySingleton (RenderSceneSingleton)),

      insertTransform (_constructor.MInsertLongTerm (Emergence::Transform::Transform3dComponent)),
      insertDynamicsMaterial (_constructor.MInsertLongTerm (Emergence::Physics::DynamicsMaterial)),
      insertRigidBody (_constructor.MInsertLongTerm (Emergence::Physics::RigidBodyComponent)),
      insertCollisionShape (_constructor.MInsertLongTerm (Emergence::Physics::CollisionShapeComponent)),

      insertCamera (_constructor.MInsertLongTerm (CameraComponent)),
      insertLight (_constructor.MInsertLongTerm (LightComponent)),
      insertStaticModel (_constructor.MInsertLongTerm (StaticModelComponent))
{
}

void SceneSeeder::Execute ()
{
    auto transformCursor = insertTransform.Execute ();
    auto materialCursor = insertDynamicsMaterial.Execute ();
    auto bodyCursor = insertRigidBody.Execute ();
    auto shapeCursor = insertCollisionShape.Execute ();

    auto cameraCursor = insertCamera.Execute ();
    auto lightCursor = insertLight.Execute ();
    auto modelCursor = insertStaticModel.Execute ();

    auto *material = static_cast<Emergence::Physics::DynamicsMaterial *> (++materialCursor);
    material->id = "Default"_us;
    material->staticFriction = 0.4f;
    material->dynamicFriction = 0.4f;
    material->enableFriction = true;
    material->restitution = 0.5f;
    material->density = 400.0f;

    auto *cameraTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (CAMERA_OBJECT_ID);
    cameraTransform->SetVisualLocalTransform (
        {{4.0f, 7.0f, -1.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *camera = static_cast<CameraComponent *> (++cameraCursor);
    camera->objectId = CAMERA_OBJECT_ID;
    camera->fieldOfViewRad = Emergence::Math::PI * 0.5f;

    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);
    renderScene->cameraObjectId = CAMERA_OBJECT_ID;

    auto *lightTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    lightTransform->SetObjectId (LIGHT_OBJECT_ID);
    lightTransform->SetVisualLocalTransform (
        {{0.0f, 5.0f, 0.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, Emergence::Math::PI / 3.0f}}, {1.0f, 1.0f, 1.0f}});

    auto *light = static_cast<LightComponent *> (++lightCursor);
    light->objectId = LIGHT_OBJECT_ID;
    light->type = LightType::DIRECTIONAL;
    light->color = {1.0f, 1.0f, 1.0f, 1.0f};

    auto *playerTransform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
    playerTransform->SetObjectId (PLAYER_OBJECT_ID);
    playerTransform->SetLogicalLocalTransform (
        {{4.0f, 7.0f, 4.0f}, Emergence::Math::Quaternion::IDENTITY, Emergence::Math::Vector3f::ONE}, true);

    auto *playerBody = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
    playerBody->objectId = PLAYER_OBJECT_ID;
    playerBody->type = Emergence::Physics::RigidBodyType::DYNAMIC;
    playerBody->linearVelocity = {2.0f, 0.0f, 2.0f};

    auto *playerShape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
    playerShape->objectId = PLAYER_OBJECT_ID;
    playerShape->shapeId = PLAYER_OBJECT_ID;
    playerShape->materialId = material->id;
    playerShape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                             .boxHalfExtents = {0.5f, 0.5f, 0.5f}};

    auto *playerModel = static_cast<StaticModelComponent *> (++modelCursor);
    playerModel->objectId = PLAYER_OBJECT_ID;
    playerModel->modelId = PLAYER_OBJECT_ID;
    playerModel->modelName = "Models/Player.mdl"_us;
    playerModel->materialNames.EmplaceBack ("Materials/Player.xml"_us);

    for (std::size_t x = 0u; x < 9u; ++x)
    {
        for (std::size_t z = 0u; z < 9u; ++z)
        {
            const Emergence::Celerity::UniqueId objectId = OTHER_OBJECTS_START_ID + x * 9u + z;

            auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
            transform->SetObjectId (objectId);
            transform->SetLogicalLocalTransform ({{static_cast<float> (x), 0.0f, static_cast<float> (z)},
                                                  Emergence::Math::Quaternion::IDENTITY,
                                                  Emergence::Math::Vector3f::ONE},
                                                 true);

            auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (++bodyCursor);
            body->objectId = objectId;
            body->type = Emergence::Physics::RigidBodyType::STATIC;

            auto *shape = static_cast<Emergence::Physics::CollisionShapeComponent *> (++shapeCursor);
            shape->objectId = objectId;
            shape->shapeId = objectId;
            shape->materialId = material->id;

            auto *model = static_cast<StaticModelComponent *> (++modelCursor);
            model->objectId = objectId;
            model->modelId = objectId;

            if (x == 0u || z == 0u || x == 8u || z == 8u)
            {
                shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                                   .boxHalfExtents = {0.5f, 1.5f, 0.5f}};
                shape->translation.y = 1.5f;

                model->modelName = "Models/Wall.mdl"_us;
                model->materialNames.EmplaceBack ("Materials/WallTileBorder.xml"_us);
                model->materialNames.EmplaceBack ("Materials/WallTileCenter.xml"_us);
            }
            else
            {
                shape->geometry = {.type = Emergence::Physics::CollisionGeometryType::BOX,
                                   .boxHalfExtents = {0.5f, 0.01f, 0.5f}};
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
    pipelineBuilder.Begin ("Seeding"_us, Emergence::Celerity::PipelineType::CUSTOM);
    pipelineBuilder.AddTask ("SceneSeeder"_us).SetExecutor<SceneSeeder> ();
    Emergence::Celerity::Pipeline *seeder = pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Input::AddToNormalUpdate (&inputAccumulator, pipelineBuilder);
    Urho3DUpdate::AddToNormalUpdate (GetContext (), pipelineBuilder);
    Emergence::Transform::VisualSync::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    // TODO: Calculate rational (for example, average parallel) amount of threads in Flow or TaskCollection?
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Input::AddToFixedUpdate (pipelineBuilder);
    Emergence::Physics::Simulation::AddToFixedUpdate (pipelineBuilder);
    Mortality::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    seeder->Execute ();
    world.RemovePipeline (seeder);
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
