#include <Application/KeyCodeMapping.hpp>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/UI.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>

#include <MainMenu/MainMenuInputResponse.hpp>

#include <MainMenuLoading/LevelsConfigurationLoading.hpp>
#include <MainMenuLoading/LoadingOrchestration.hpp>
#include <MainMenuLoading/MainMenuInitialization.hpp>

#include <Modules/MainMenu.hpp>

#include <Render/Backend/Configuration.hpp>

using namespace Emergence::Memory::Literals;

namespace Modules::MainMenu
{
Emergence::Memory::UniqueString GetName () noexcept
{
    static Emergence::Memory::UniqueString name {"MainMenu"_us};
    return name;
}

Emergence::Celerity::WorldViewConfig GetViewConfig () noexcept
{
    static Emergence::Celerity::WorldViewConfig rootViewConfig = [] ()
    {
        Emergence::Celerity::WorldViewConfig config;
        return config;
    }();

    return rootViewConfig;
}

void Initializer (GameState &_gameState,
                  Emergence::Celerity::World &_world,
                  Emergence::Celerity::WorldView &_view) noexcept
{
    Emergence::Celerity::PipelineBuilder mainPipelineBuilder {&_view};
    mainPipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Emergence::Celerity::Input::AddToFixedUpdate (mainPipelineBuilder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (mainPipelineBuilder);
    [[maybe_unused]] const bool fixedPipelineRegistered = mainPipelineBuilder.End ();
    EMERGENCE_ASSERT (fixedPipelineRegistered);

    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

    mainPipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);

    // Mock asset management checkpoints.
    mainPipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::STARTED);
    mainPipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    mainPipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
    mainPipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::FINISHED);

    Emergence::Celerity::Input::AddToNormalUpdate (mainPipelineBuilder, _gameState.GetFrameInputAccumulator ());
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (mainPipelineBuilder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (mainPipelineBuilder, worldBox);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (mainPipelineBuilder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (mainPipelineBuilder);
    Emergence::Celerity::UI::AddToNormalUpdate (mainPipelineBuilder, _gameState.GetFrameInputAccumulator (),
                                                GetKeyCodeMapping ());
    MainMenuInputResponse::AddToNormalUpdate (mainPipelineBuilder, _gameState.ConstructWorldStateRedirectionHandle ());
    [[maybe_unused]] const bool normalPipelineRegistered = mainPipelineBuilder.End ();
    EMERGENCE_ASSERT (normalPipelineRegistered);

    Emergence::Celerity::WorldViewConfig loadingViewConfig;
    Emergence::Celerity::WorldView *loadingView = _world.CreateView (&_view, "Loading"_us, loadingViewConfig);

    Emergence::Celerity::PipelineBuilder loadingPipelineBuilder {loadingView};
    loadingPipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    LevelsConfigurationLoading::AddToNormalUpdate (loadingPipelineBuilder);
    LoadingAnimation::AddToNormalUpdate (loadingPipelineBuilder);
    MainMenuInitialization::AddToNormalUpdate (loadingPipelineBuilder);
    MainMenuLoadingOrchestration::AddToNormalUpdate (loadingPipelineBuilder, _gameState.ConstructViewDropHandle (),
                                                     loadingView);
    [[maybe_unused]] const bool loadingNormalPipelineRegistered = loadingPipelineBuilder.End ();
    EMERGENCE_ASSERT (loadingNormalPipelineRegistered);
}
} // namespace Modules::MainMenu
