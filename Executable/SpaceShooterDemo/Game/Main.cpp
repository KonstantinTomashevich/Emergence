#define _CRT_SECURE_NO_WARNINGS

#include <SyntaxSugar/MuteWarnings.hpp>

#include <fstream>

#include <Assembly/AssemblerConfiguration.hpp>

#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Physics3d/Events.hpp>
#include <Celerity/Physics3d/Simulation.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Config/PathMappingLoading.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/World.hpp>

#include <Export/Graph.hpp>

#include <Gameplay/Control.hpp>
#include <Gameplay/Damage.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/FollowCamera.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/Movement.hpp>
#include <Gameplay/NonFeatureSpecificComponentCleanup.hpp>
#include <Gameplay/RandomAi.hpp>
#include <Gameplay/Shooting.hpp>
#include <Gameplay/Slowdown.hpp>
#include <Gameplay/Spawn.hpp>

#include <Loading/Model/ResourceConfigTypeMeta.hpp>
#include <Loading/Model/ResourceObjectTypeManifest.hpp>
#include <Loading/Task/InputInitialization.hpp>
#include <Loading/Task/LevelGeneration.hpp>
#include <Loading/Task/LoadingOrchestration.hpp>
#include <Loading/Task/PhysicsInitialization.hpp>

#include <Log/Log.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Recording/StreamSerializer.hpp>

#include <Render/Events.hpp>
#include <Render/Urho3DUpdate.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#undef ERROR

using namespace Emergence::Memory::Literals;

static Emergence::Memory::Profiler::EventObserver StartRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

static void SaveVisualGraph (const Emergence::VisualGraph::Graph &_graph, const Emergence::Container::String &_fileName)
{
    std::ofstream output (_fileName.c_str ());
    Emergence::Export::Graph::Export (_graph, output);
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

    Emergence::Celerity::FrameInputAccumulator inputAccumulator;
    Emergence::Celerity::World world {"TestWorld"_us, {{1.0f / 60.0f}}};

    Emergence::Celerity::Pipeline *loadingPipeline = nullptr;
    bool loadingFinished = false;
};

GameApplication::GameApplication (Urho3D::Context *_context)
    : Application (_context),
      memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartRecording (memoryEventSerializer, memoryEventOutput))
{
    Emergence::SetIsAssertInteractive (true);
    SubscribeToEvent (Urho3D::E_UPDATE, URHO3D_HANDLER (GameApplication, HandleUpdate));
    SubscribeToEvent (Urho3D::E_KEYDOWN, URHO3D_HANDLER (GameApplication, HandleKeyDown));
    SubscribeToEvent (Urho3D::E_KEYUP, URHO3D_HANDLER (GameApplication, HandleKeyUp));

    Emergence::Log::GlobalLogger::Init (Emergence::Log::Level::ERROR,
                                        {Emergence::Log::Sinks::StandardOut {{Emergence::Log::Level::INFO}}});
}

void GameApplication::Setup ()
{
    Application::Setup ();
    engineParameters_[Urho3D::EP_FULL_SCREEN] = false;
    engineParameters_[Urho3D::EP_WINDOW_RESIZABLE] = true;
    engineParameters_[Urho3D::EP_WINDOW_TITLE] = "Emergence Space Shooter Demo";
    engineParameters_[Urho3D::EP_RESOURCE_PATHS] = "Urho3DCoreResources;GameResources";
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
        Emergence::Celerity::RegisterAssemblyEvents (registrar);
        Emergence::Celerity::RegisterPhysicsEvents (registrar);
        Emergence::Celerity::RegisterTransform3dEvents (registrar);
        RegisterGameplayEvents (registrar);
        RegisterRenderEvents (registrar);
    }

    Emergence::VisualGraph::Graph pipelineVisualGraph;

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};
    pipelineBuilder.Begin ("Loading"_us, Emergence::Celerity::PipelineType::CUSTOM);
    Emergence::Celerity::ResourceConfigLoading::AddToLoadingPipeline (pipelineBuilder, 16000000u /*16 ms*/,
                                                                      PrepareResourceConfigTypeMeta ());
    Emergence::Celerity::ResourceConfigPathMappingLoading::AddToLoadingPipeline (pipelineBuilder, "../GameResources",
                                                                                 PrepareResourceConfigTypeMeta ());
    Emergence::Celerity::ResourceObjectLoading::AddToLoadingPipeline (pipelineBuilder,
                                                                      PrepareResourceObjectTypeManifest ());
    InputInitialization::AddToLoadingPipeline (pipelineBuilder);
    LevelGeneration::AddToLoadingPipeline (pipelineBuilder);
    PhysicsInitialization::AddToLoadingPipeline (pipelineBuilder);
    LoadingOrchestration::AddToLoadingPipeline (pipelineBuilder, &loadingFinished);
    loadingPipeline = pipelineBuilder.End (&pipelineVisualGraph);
    SaveVisualGraph (pipelineVisualGraph, "LoadingPipeline.graph");

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Control::AddToFixedUpdate (pipelineBuilder);
    Damage::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::Assembly::AddToFixedUpdate (pipelineBuilder, GetAssemblerCustomKeys (),
                                                     GetFixedAssemblerTypes ());
    Emergence::Celerity::Input::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::TransformHierarchyCleanup::Add3dToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::Physics3dSimulation::AddToFixedUpdate (pipelineBuilder);
    Mortality::AddToFixedUpdate (pipelineBuilder);
    Movement::AddToFixedUpdate (pipelineBuilder);
    NonFeatureSpecificComponentCleanup::AddToFixedUpdate (pipelineBuilder);
    RandomAi::AddToFixedUpdate (pipelineBuilder);
    Shooting::AddToFixedUpdate (pipelineBuilder);
    Slowdown::AddToFixedUpdate (pipelineBuilder);
    Spawn::AddToFixedUpdate (pipelineBuilder);
    pipelineBuilder.End (&pipelineVisualGraph);
    SaveVisualGraph (pipelineVisualGraph, "FixedPipeline.graph");

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Emergence::Celerity::Assembly::AddToNormalUpdate (pipelineBuilder, GetAssemblerCustomKeys (),
                                                      GetNormalAssemblerTypes ());
    Emergence::Celerity::Input::AddToNormalUpdate (pipelineBuilder, &inputAccumulator);
    Emergence::Celerity::TransformHierarchyCleanup::Add3dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::TransformVisualSync::Add3dToNormalUpdate (pipelineBuilder);
    FollowCamera::AddToNormalUpdate (pipelineBuilder);
    Mortality::AddToNormalUpdate (pipelineBuilder);
    Urho3DUpdate::AddToNormalUpdate (GetContext (), pipelineBuilder);
    pipelineBuilder.End (&pipelineVisualGraph);
    SaveVisualGraph (pipelineVisualGraph, "NormalPipeline.graph");
}

void GameApplication::Stop ()
{
    Application::Stop ();
}

void GameApplication::HandleUpdate (Urho3D::StringHash /*unused*/, Urho3D::VariantMap & /*unused*/) noexcept
{
    if (loadingPipeline)
    {
        loadingPipeline->Execute ();
        if (loadingFinished)
        {
            world.RemovePipeline (loadingPipeline);
            loadingPipeline = nullptr;
        }
    }

    world.Update ();
    inputAccumulator.Clear ();

    while (const Emergence::Memory::Profiler::Event *event = memoryEventObserver.NextEvent ())
    {
        memoryEventSerializer.SerializeEvent (*event);
    }
}

void GameApplication::HandleKeyDown (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    inputAccumulator.RecordEvent (Emergence::Celerity::InputEvent {
        Emergence::Time::NanosecondsSinceStartup (),
        Emergence::Celerity::KeyboardEvent {
            static_cast<Emergence::Celerity::KeyCode> (_eventData[Urho3D::KeyDown::P_KEY].GetInt ()),
            static_cast<Emergence::Celerity::ScanCode> (_eventData[Urho3D::KeyDown::P_SCANCODE].GetInt ()),
            static_cast<Emergence::Celerity::QualifiersMask> (_eventData[Urho3D::KeyDown::P_QUALIFIERS].GetInt ()),
            Emergence::Celerity::KeyState::DOWN, _eventData[Urho3D::KeyDown::P_REPEAT].GetBool ()}});
}

void GameApplication::HandleKeyUp (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    inputAccumulator.RecordEvent (Emergence::Celerity::InputEvent {
        Emergence::Time::NanosecondsSinceStartup (),
        Emergence::Celerity::KeyboardEvent {
            static_cast<Emergence::Celerity::KeyCode> (_eventData[Urho3D::KeyDown::P_KEY].GetInt ()),
            static_cast<Emergence::Celerity::ScanCode> (_eventData[Urho3D::KeyDown::P_SCANCODE].GetInt ()),
            static_cast<Emergence::Celerity::QualifiersMask> (_eventData[Urho3D::KeyDown::P_QUALIFIERS].GetInt ()),
            Emergence::Celerity::KeyState::UP, _eventData[Urho3D::KeyDown::P_REPEAT].GetBool ()}});
}

BEGIN_MUTING_WARNINGS
URHO3D_DEFINE_APPLICATION_MAIN (GameApplication)
END_MUTING_WARNINGS
