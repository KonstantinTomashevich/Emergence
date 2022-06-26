#define _CRT_SECURE_NO_WARNINGS

#include <SyntaxSugar/MuteWarnings.hpp>

#include <fstream>
#include <thread>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Gameplay/Assembly.hpp>
#include <Gameplay/Control.hpp>
#include <Gameplay/Damage.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/FollowCamera.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/Movement.hpp>
#include <Gameplay/RandomAi.hpp>
#include <Gameplay/Shooting.hpp>
#include <Gameplay/Slowdown.hpp>
#include <Gameplay/Spawn.hpp>

#include <Initialization/InputInitialization.hpp>
#include <Initialization/LevelGeneration.hpp>
#include <Initialization/PhysicsInitialization.hpp>

#include <Input/Input.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Recording/StreamSerializer.hpp>

#include <Physics/Events.hpp>
#include <Physics/Simulation.hpp>

#include <Render/Events.hpp>
#include <Render/Urho3DUpdate.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dVisualSync.hpp>

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

    InputAccumulator inputAccumulator;
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
    engineParameters_[Urho3D::EP_WINDOW_TITLE] = "Emergence Space Shooter Demo";
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
        Emergence::Celerity::RegisterPhysicsEvents (registrar);
        Emergence::Celerity::RegisterTransformEvents (registrar);
        RegisterGameplayEvents (registrar);
        RegisterRenderEvents (registrar);
    }

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};
    pipelineBuilder.Begin ("Initialization"_us, Emergence::Celerity::PipelineType::CUSTOM);
    InputInitialization::AddToInitializationPipeline (pipelineBuilder);
    LevelGeneration::AddToInitializationPipeline (pipelineBuilder);
    PhysicsInitialization::AddToInitializationPipeline (pipelineBuilder);
    Emergence::Celerity::Pipeline *initializer = pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Assembly::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    Emergence::Celerity::VisualTransformSync::AddToNormalUpdate (pipelineBuilder);
    FollowCamera::AddToNormalUpdate (pipelineBuilder);
    Input::AddToNormalUpdate (&inputAccumulator, pipelineBuilder);
    Mortality::AddToNormalUpdate (pipelineBuilder);
    Urho3DUpdate::AddToNormalUpdate (GetContext (), pipelineBuilder);
    // TODO: Calculate rational (for example, average parallel) amount of threads in Flow or TaskCollection?
    pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Assembly::AddToFixedUpdate (pipelineBuilder);
    Control::AddToFixedUpdate (pipelineBuilder);
    Damage::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
    Emergence::Celerity::Simulation::AddToFixedUpdate (pipelineBuilder);
    Input::AddToFixedUpdate (pipelineBuilder);
    Mortality::AddToFixedUpdate (pipelineBuilder);
    Movement::AddToFixedUpdate (pipelineBuilder);
    RandomAi::AddToFixedUpdate (pipelineBuilder);
    Shooting::AddToFixedUpdate (pipelineBuilder);
    Slowdown::AddToFixedUpdate (pipelineBuilder);
    Spawn::AddToFixedUpdate (pipelineBuilder);
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

    InputEvent event;
    event.type = InputType::KEYBOARD;
    event.keyboard = {
        static_cast<ScanCode> (_eventData[Urho3D::KeyDown::P_SCANCODE].GetInt ()),
        static_cast<KeyCode> (_eventData[Urho3D::KeyDown::P_KEY].GetInt ()),
        true,
        static_cast<QualifiersMask> (_eventData[Urho3D::KeyDown::P_QUALIFIERS].GetInt ()),
    };

    inputAccumulator.PostEvent (event);
}

void GameApplication::HandleKeyUp (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_eventData) noexcept
{
    InputEvent event;
    event.type = InputType::KEYBOARD;
    event.keyboard = {
        static_cast<ScanCode> (_eventData[Urho3D::KeyUp::P_SCANCODE].GetInt ()),
        static_cast<KeyCode> (_eventData[Urho3D::KeyUp::P_KEY].GetInt ()),
        false,
        static_cast<QualifiersMask> (_eventData[Urho3D::KeyUp::P_QUALIFIERS].GetInt ()),
    };

    inputAccumulator.PostEvent (event);
}

BEGIN_MUTING_WARNINGS
URHO3D_DEFINE_APPLICATION_MAIN (GameApplication)
END_MUTING_WARNINGS
