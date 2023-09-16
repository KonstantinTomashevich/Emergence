#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimationManagement.hpp>
#include <Celerity/Asset/Render/Foundation/FrameBufferManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Nexus/Nexus.hpp>
#include <Celerity/Physics2d/DebugDraw.hpp>
#include <Celerity/Physics2d/Simulation.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/Foundation/PostProcess.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/UI.hpp>

#include <Configuration/AssemblyConfiguration.hpp>
#include <Configuration/ResourceConfigTypeMeta.hpp>
#include <Configuration/ResourceObjectTypeManifest.hpp>

#include <Core/KeyCodeMapping.hpp>
#include <Core/NexusUserContext.hpp>

#include <GameCore/GameStateManagement.hpp>
#include <GameCore/LevelsConfigurationLoading.hpp>

#include <LevelLoading/LevelLoading.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>

#include <MainMenu/MainMenuManagement.hpp>

#include <MainMenuLoading/LoadingOrchestration.hpp>

#include <Platformer/Animation/CharacterAnimation.hpp>
#include <Platformer/Camera/Camera.hpp>
#include <Platformer/Control/PlayerControl.hpp>
#include <Platformer/Layer/LayerSetup.hpp>
#include <Platformer/LooseCascadeRemovers.hpp>
#include <Platformer/Movement/Movement.hpp>
#include <Platformer/PlatformerManagement.hpp>
#include <Platformer/Spawn/Spawn.hpp>
#include <PlatformerLoading/LoadingOrchestration.hpp>

namespace
{
constexpr std::uint64_t MAX_ASSEMBLY_TIME_NS = 10000000u;

const Emergence::Math::AxisAlignedBox2d WORLD_BOX {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

void MockRootCheckpoints (Emergence::Celerity::PipelineBuilder &_builder)
{
    _builder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::STARTED);
    _builder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _builder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
    _builder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::FINISHED);
    _builder.AddCheckpoint (Emergence::Celerity::Localization::Checkpoint::SYNC_STARTED);
    _builder.AddCheckpoint (Emergence::Celerity::Localization::Checkpoint::SYNC_FINISHED);
}
} // namespace

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineRootNormal (
    Emergence::Celerity::NexusNode *_node, Emergence::Celerity::PipelineBuilder &_builder)
{
    auto *context = static_cast<NexusUserContext *> (_node->GetNexus ()->GetUserContext ());
    const auto *bindingList =
        static_cast<const Emergence::Celerity::AssetReferenceBindingList *> (context->assetReferenceBindingList.Get ());
    const auto *eventMap = static_cast<const Emergence::Celerity::AssetReferenceBindingEventMap *> (
        context->assetReferenceBindingEventMap.Get ());

    Emergence::Celerity::AssetManagement::AddToNormalUpdate (_builder, *bindingList, *eventMap);
    Emergence::Celerity::FontManagement::AddToNormalUpdate (_builder, &context->resourceProvider.value (), *eventMap);
    Emergence::Celerity::FrameBufferManagement::AddToNormalUpdate (_builder);
    Emergence::Celerity::Localization::AddToNormalUpdate (_builder, &context->resourceProvider.value ());
    Emergence::Celerity::MaterialInstanceManagement::AddToNormalUpdate (_builder, &context->resourceProvider.value (),
                                                                        *eventMap);
    Emergence::Celerity::MaterialManagement::AddToNormalUpdate (_builder, &context->resourceProvider.value (),
                                                                *eventMap);
    Emergence::Celerity::ResourceConfigLoading::AddToLoadingPipeline (_builder, &context->resourceProvider.value (),
                                                                      GetResourceConfigTypeMeta ());
    Emergence::Celerity::ResourceObjectLoading::AddToLoadingPipeline (_builder, &context->resourceProvider.value (),
                                                                      GetResourceObjectTypeManifest ());
    Emergence::Celerity::Sprite2dUvAnimationManagement::AddToNormalUpdate (
        _builder, &context->resourceProvider.value (), *eventMap);
    Emergence::Celerity::TextureManagement::AddToNormalUpdate (_builder, &context->resourceProvider.value (),
                                                               *eventMap);

    _builder.AddCheckpointDependency (Emergence::Celerity::ResourceConfigLoading::Checkpoint::FINISHED,
                                      Emergence::Celerity::AssetManagement::Checkpoint::STARTED);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineGameRootNormal (
    Emergence::Celerity::NexusNode *_node, Emergence::Celerity::PipelineBuilder &_builder)
{
    auto *context = static_cast<NexusUserContext *> (_node->GetNexus ()->GetUserContext ());
    GameStateManagement::AddToNormalUpdate (_builder, context->gameWorldNode, &context->terminateRequested);
    LevelsConfigurationLoading::AddToNormalUpdate (_builder, &context->resourceProvider.value ());
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineMainMenuLoadingFixed (
    Emergence::Celerity::NexusNode * /*unused*/, Emergence::Celerity::PipelineBuilder &_builder)
{
    Emergence::Celerity::Assembly::AddToFixedUpdate (_builder, GetAssemblerCustomKeys (), GetFixedAssemblerTypes (),
                                                     MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::Input::AddToFixedUpdate (_builder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (_builder);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineMainMenuLoadingNormal (
    Emergence::Celerity::NexusNode * /*unused*/, Emergence::Celerity::PipelineBuilder &_builder)
{
    MockRootCheckpoints (_builder);
    Emergence::Celerity::Assembly::AddToNormalUpdate (_builder, GetAssemblerCustomKeys (), GetNormalAssemblerTypes (),
                                                      MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (_builder, WORLD_BOX);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (_builder);
    LevelLoading::AddToNormalUpdate (_builder);
    LoadingAnimation::AddToNormalUpdate (_builder);
    MainMenuLoadingOrchestration::AddToNormalUpdate (_builder);

    _builder.AddCheckpointDependency (Emergence::Celerity::Assembly::Checkpoint::FINISHED,
                                      Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineMainMenuReadyFixed (
    Emergence::Celerity::NexusNode *_node, Emergence::Celerity::PipelineBuilder &_builder)
{
    BuildPipelineMainMenuLoadingFixed (_node, _builder);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelineMainMenuReadyNormal (
    Emergence::Celerity::NexusNode *_node, Emergence::Celerity::PipelineBuilder &_builder)
{
    auto *context = static_cast<NexusUserContext *> (_node->GetNexus ()->GetUserContext ());
    MockRootCheckpoints (_builder);

    Emergence::Celerity::Assembly::AddToNormalUpdate (_builder, GetAssemblerCustomKeys (), GetNormalAssemblerTypes (),
                                                      MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::Input::AddToNormalUpdate (_builder, &context->inputAccumulator);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (_builder, WORLD_BOX);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::UI::AddToNormalUpdate (_builder, &context->inputAccumulator, GetKeyCodeMapping ());
    MainMenuManagement::AddToNormalUpdate (_builder);

    _builder.AddCheckpointDependency (Emergence::Celerity::Assembly::Checkpoint::FINISHED,
                                      Emergence::Celerity::UI::Checkpoint::HIERARCHY_CLEANUP_STARTED);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelinePlatformerLoadingFixed (
    Emergence::Celerity::NexusNode * /*unused*/, Emergence::Celerity::PipelineBuilder &_builder)
{
    // Mock input checkpoints for PlayerControl.
    _builder.AddCheckpoint (Emergence::Celerity::Input::Checkpoint::ACTION_DISPATCH_STARTED);
    _builder.AddCheckpoint (Emergence::Celerity::Input::Checkpoint::CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED);
    _builder.AddCheckpoint (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);

    Emergence::Celerity::Assembly::AddToFixedUpdate (_builder, GetAssemblerCustomKeys (), GetFixedAssemblerTypes (),
                                                     MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::Physics2dSimulation::AddToFixedUpdate (_builder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (_builder);
    LooseCascadeRemovers::AddToFixedPipeline (_builder);
    PlayerControl::AddToFixedUpdate (_builder); // Needed to react to player spawn during loading.
    Spawn::AddToFixedUpdate (_builder);         // Needed to spawn initial level mobs during loading.

    _builder.AddCheckpointDependency (Emergence::Celerity::Assembly::Checkpoint::FINISHED,
                                      Emergence::Celerity::Physics2dSimulation::Checkpoint::STARTED);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelinePlatformerLoadingNormal (
    Emergence::Celerity::NexusNode * /*unused*/, Emergence::Celerity::PipelineBuilder &_builder)
{
    MockRootCheckpoints (_builder);
    CharacterAnimation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Assembly::AddToNormalUpdate (_builder, GetAssemblerCustomKeys (), GetNormalAssemblerTypes (),
                                                      MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::PostProcess::AddToNormalUpdate (_builder);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (_builder, WORLD_BOX);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (_builder);
    LayerSetup::AddToNormalUpdate (_builder);
    LevelLoading::AddToNormalUpdate (_builder);
    LoadingAnimation::AddToNormalUpdate (_builder);
    PlatformerLoadingOrchestration::AddToNormalUpdate (_builder);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelinePlatformerGameFixed (
    Emergence::Celerity::NexusNode * /*unused*/, Emergence::Celerity::PipelineBuilder &_builder)
{
    Emergence::Celerity::Assembly::AddToFixedUpdate (_builder, GetAssemblerCustomKeys (), GetFixedAssemblerTypes (),
                                                     MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::Input::AddToFixedUpdate (_builder);
    Emergence::Celerity::Physics2dDebugDraw::AddToFixedUpdate (_builder);
    Emergence::Celerity::Physics2dSimulation::AddToFixedUpdate (_builder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (_builder);
    LooseCascadeRemovers::AddToFixedPipeline (_builder);
    Movement::AddToFixedUpdate (_builder);
    PlayerControl::AddToFixedUpdate (_builder);
    Spawn::AddToFixedUpdate (_builder);

    _builder.AddCheckpointDependency (Emergence::Celerity::Assembly::Checkpoint::FINISHED,
                                      Emergence::Celerity::Physics2dSimulation::Checkpoint::STARTED);
}

Platformer2dDemoLogicApi extern "C" __cdecl void BuildPipelinePlatformerGameNormal (
    Emergence::Celerity::NexusNode *_node, Emergence::Celerity::PipelineBuilder &_builder)
{
    auto *context = static_cast<NexusUserContext *> (_node->GetNexus ()->GetUserContext ());
    MockRootCheckpoints (_builder);

    CharacterAnimation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Assembly::AddToNormalUpdate (_builder, GetAssemblerCustomKeys (), GetNormalAssemblerTypes (),
                                                      MAX_ASSEMBLY_TIME_NS);
    Emergence::Celerity::Input::AddToNormalUpdate (_builder, &context->inputAccumulator);
    Emergence::Celerity::PostProcess::AddToNormalUpdate (_builder);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (_builder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (_builder, WORLD_BOX);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (_builder);
    Emergence::Celerity::UI::AddToNormalUpdate (_builder, &context->inputAccumulator, GetKeyCodeMapping ());
    Camera::AddToNormalUpdate (_builder);
    LayerSetup::AddToNormalUpdate (_builder);
    PlatformerManagement::AddToNormalUpdate (_builder);

    _builder.AddCheckpointDependency (Emergence::Celerity::Assembly::Checkpoint::FINISHED,
                                      Emergence::Celerity::UI::Checkpoint::HIERARCHY_CLEANUP_STARTED);
}
