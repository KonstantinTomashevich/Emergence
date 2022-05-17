#include <SyntaxSugar/MuteWarnings.hpp>

#include <fstream>
#include <thread>

#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Input/Input.hpp>

#include <Log/Log.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Recording/StreamSerializer.hpp>

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxPhysicsVersion.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxScene.h>
#include <common/PxTolerancesScale.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxRigidBodyExt.h>
#include <extensions/PxSimpleFactory.h>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>

#include <Shared/CelerityUtils.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#undef ERROR

using namespace Emergence::Memory::Literals;

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

    ~GameApplication () override;

    void Setup () override;

    void Start () override;

    void Stop () override;

    GameApplication &operator= (const GameApplication &_other) = delete;

    GameApplication &operator= (GameApplication &&_other) = delete;

private:
    void HandleUpdate (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept;

    void HandleKeyDown (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept;

    void HandleKeyUp (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept;

    std::ofstream memoryEventOutput;
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;
    Emergence::Memory::Profiler::EventObserver memoryEventObserver;

    Emergence::Celerity::InputAccumulator inputAccumulator;
    Emergence::Celerity::World world {"TestWorld"_us};

    Urho3D::SharedPtr<Urho3D::Scene> scene;
    Urho3D::SharedPtr<Urho3D::Node> playerNode;
    Urho3D::SharedPtr<Urho3D::Node> lightNode;

    physx::PxDefaultAllocator gAllocator;
    physx::PxDefaultErrorCallback gErrorCallback;

    physx::PxFoundation *gFoundation = nullptr;
    physx::PxPhysics *gPhysics = nullptr;

    physx::PxDefaultCpuDispatcher *gDispatcher = nullptr;
    physx::PxScene *gScene = nullptr;

    physx::PxMaterial *gMaterial = nullptr;

    physx::PxPvd *gPvd = nullptr;

    physx::PxRigidDynamic *boxBody = nullptr;
};

GameApplication::GameApplication (Urho3D::Context *_context)
    : Application (_context),
      memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartRecording (memoryEventSerializer, memoryEventOutput))
{
    SubscribeToEvent (Urho3D::E_UPDATE, URHO3D_HANDLER (GameApplication, HandleUpdate));
    SubscribeToEvent (Urho3D::E_KEYDOWN, URHO3D_HANDLER (GameApplication, HandleKeyDown));
    SubscribeToEvent (Urho3D::E_KEYUP, URHO3D_HANDLER (GameApplication, HandleKeyUp));

    gFoundation = PxCreateFoundation (PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPvd = physx::PxCreatePvd (*gFoundation);
    physx::PxPvdTransport *transport = physx::PxDefaultPvdSocketTransportCreate ("127.0.0.1", 5425, 10);
    gPvd->connect (*transport, physx::PxPvdInstrumentationFlag::eALL);
    gPhysics = PxCreatePhysics (PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale (), true, gPvd);
}

GameApplication::~GameApplication ()
{
    gScene->release ();
    gDispatcher->release ();
    gPhysics->release ();

    if (gPvd)
    {
        physx::PxPvdTransport *transport = gPvd->getTransport ();
        gPvd->release ();
        transport->release ();
    }

    gFoundation->release ();
}

void GameApplication::Setup ()
{
    Application::Setup ();
    engineParameters_[Urho3D::EP_FULL_SCREEN] = false;
    engineParameters_[Urho3D::EP_RESOURCE_PATHS] = "Urho3DCoreAssets;GameAssets";
    engineParameters_[Urho3D::EP_RESOURCE_PREFIX_PATHS] = "..";
}

void GameApplication::Start ()
{
    Application::Start ();
    auto *input = GetSubsystem<Urho3D::Input> ();
    input->SetMouseVisible (true);
    input->SetMouseMode (Urho3D::MM_FREE);

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};
    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Input::AddToNormalUpdate (&inputAccumulator, pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);

    // TODO: Calculate rational (for example, average parallel) amount of threads in Flow or TaskCollection?
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Input::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    scene = new Urho3D::Scene {GetContext ()};
    scene->SetUpdateEnabled (false);
    scene->CreateComponent<Urho3D::Octree> ();
    auto *cache = GetSubsystem<Urho3D::ResourceCache> ();

    auto *floorTileXml = cache->GetResource<Urho3D::XMLFile> ("Objects/FloorTile.xml");
    if (!floorTileXml)
    {
        EMERGENCE_LOG (ERROR, "Unable to load floor tile prefab!");
    }

    auto *playerXml = cache->GetResource<Urho3D::XMLFile> ("Objects/Player.xml");
    if (!playerXml)
    {
        EMERGENCE_LOG (ERROR, "Unable to load player prefab!");
    }

    auto *wallXml = cache->GetResource<Urho3D::XMLFile> ("Objects/Wall.xml");
    if (!wallXml)
    {
        EMERGENCE_LOG (ERROR, "Unable to load wall prefab!");
    }

    for (int x = -4; x <= 4; ++x)
    {
        for (int z = -4; z <= 4; ++z)
        {
            const bool placeWall = x == -4 || x == 4 || z == -4 || z == 4;
            Urho3D::XMLFile *prefabXml = placeWall ? wallXml : floorTileXml;

            if (prefabXml)
            {
                Urho3D::Node *node = scene->CreateChild (Urho3D::String::EMPTY, Urho3D::LOCAL);
                node->LoadXML (prefabXml->GetRoot ());
                node->SetPosition ({static_cast<float> (x), 0.0f, static_cast<float> (z)});
            }
        }
    }

    if (playerXml)
    {
        playerNode = scene->CreateChild (Urho3D::String::EMPTY, Urho3D::LOCAL);
        playerNode->LoadXML (playerXml->GetRoot ());
        playerNode->SetPosition ({});
    }

    lightNode = scene->CreateChild (Urho3D::String::EMPTY, Urho3D::LOCAL);
    auto *light = lightNode->CreateComponent<Urho3D::Light> (Urho3D::LOCAL);
    light->SetLightType (Urho3D::LIGHT_DIRECTIONAL);
    lightNode->SetRotation ({30.0f, 0.0f, 0.0f});

    Urho3D::Node *cameraNode = scene->CreateChild (Urho3D::String::EMPTY, Urho3D::LOCAL);
    auto *camera = cameraNode->CreateComponent<Urho3D::Camera> ();
    cameraNode->SetPosition ({0.0f, 10.0f, -5.0f});
    cameraNode->SetRotation ({60.0f, 0.0f, 0.0f});

    auto *renderer = GetSubsystem<Urho3D::Renderer> ();
    renderer->SetViewport (0, new Urho3D::Viewport {GetContext (), scene, camera});

    physx::PxSceneDesc sceneDesc (gPhysics->getTolerancesScale ());
    sceneDesc.gravity = physx::PxVec3 (0.0f, -9.81f, 0.0f);
    gDispatcher = physx::PxDefaultCpuDispatcherCreate (2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene (sceneDesc);

    physx::PxPvdSceneClient *pvdClient = gScene->getScenePvdClient ();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag (physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag (physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag (physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    gMaterial = gPhysics->createMaterial (0.5f, 0.5f, 0.6f);
    physx::PxRigidStatic *groundPlane =
        physx::PxCreatePlane (*gPhysics, physx::PxPlane (0.0f, 1.0f, 0.0f, 0.0f), *gMaterial);
    gScene->addActor (*groundPlane);

    boxBody = gPhysics->createRigidDynamic (physx::PxTransform (0.0f, 10.0f, 0.0f));
    physx::PxShape *boxShape = gPhysics->createShape (physx::PxBoxGeometry (0.5f, 0.5f, 0.5f), *gMaterial);
    boxBody->attachShape (*boxShape);
    physx::PxRigidBodyExt::updateMassAndInertia (*boxBody, 1000.0f);
    gScene->addActor (*boxBody);
    boxShape->release ();
}

void GameApplication::Stop ()
{
    Application::Stop ();
}

void GameApplication::HandleUpdate (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    world.Update ();

    gScene->simulate (_eventData[Urho3D::Update::P_TIMESTEP].GetFloat ());
    gScene->fetchResults (true);
    const physx::PxTransform boxTransform = boxBody->getGlobalPose ();

    constexpr const uint64_t NS_PERIOD = 3000000000u;
    const uint64_t nsGlobal = Emergence::Time::NanosecondsSinceStartup ();
    const uint64_t nsLocal = nsGlobal % NS_PERIOD;
    const float angle = M_PI * 2.0f * (static_cast<float> (nsLocal) / static_cast<float> (NS_PERIOD));

    lightNode->LookAt ({cos (angle), -1.0f, sin (angle)});
    // playerNode->SetPosition ({cos (angle) * 2.0f, 0.0f, sin (angle) * 2.0f});
    playerNode->SetPosition ({boxTransform.p.x, boxTransform.p.y, boxTransform.p.z});
    playerNode->SetRotation ({boxTransform.q.w, boxTransform.q.x, boxTransform.q.y, boxTransform.q.z});

    scene->Update (_eventData[Urho3D::Update::P_TIMESTEP].GetFloat ());
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
