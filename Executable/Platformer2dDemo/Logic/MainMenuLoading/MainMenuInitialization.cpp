#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/UI/UISingleton.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Configuration/VisibilityMask.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>

#include <MainMenu/InputActions.hpp>
#include <MainMenu/MainMenuSingleton.hpp>

#include <MainMenuLoading/LevelsConfigurationLoading.hpp>
#include <MainMenuLoading/MainMenuInitialization.hpp>
#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

namespace MainMenuInitialization
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"MainMenuInitialization::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"MainMenuInitialization::Finished"};

class Initializer final : public Emergence::Celerity::TaskExecutorBase<Initializer>
{
public:
    Initializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchRender;
    Emergence::Celerity::FetchSingletonQuery fetchUI;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenu;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenuLoading;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertSprite;
    Emergence::Celerity::InsertLongTermQuery insertInputSubscription;

    Emergence::Celerity::InsertLongTermQuery insertUINode;
    Emergence::Celerity::InsertLongTermQuery insertWindowControl;
    Emergence::Celerity::InsertLongTermQuery insertButtonControl;
    Emergence::Celerity::InsertLongTermQuery insertFontProperty;

    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;
};

Initializer::Initializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchRender (FETCH_SINGLETON (Emergence::Celerity::Render2dSingleton)),
      fetchUI (FETCH_SINGLETON (Emergence::Celerity::UISingleton)),
      modifyMainMenu (MODIFY_SINGLETON (MainMenuSingleton)),
      modifyMainMenuLoading (MODIFY_SINGLETON (MainMenuLoadingSingleton)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertSprite (INSERT_LONG_TERM (Emergence::Celerity::Sprite2dComponent)),
      insertInputSubscription (INSERT_LONG_TERM (Emergence::Celerity::InputSubscriptionComponent)),

      insertUINode (INSERT_LONG_TERM (Emergence::Celerity::UINode)),
      insertWindowControl (INSERT_LONG_TERM (Emergence::Celerity::WindowControl)),
      insertButtonControl (INSERT_LONG_TERM (Emergence::Celerity::ButtonControl)),
      insertFontProperty (INSERT_LONG_TERM (Emergence::Celerity::UIStyleFontProperty)),

      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (LoadingAnimation::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (LevelsConfigurationLoading::Checkpoint::STARTED);
}

void Initializer::Execute () noexcept
{
    auto mainMenuLoadingCursor = modifyMainMenuLoading.Execute ();
    auto *mainMenuLoading = static_cast<MainMenuLoadingSingleton *> (*mainMenuLoadingCursor);

    if (!mainMenuLoading->mainMenuInitialized)
    {
        auto worldCursor = fetchWorld.Execute ();
        const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

        auto renderCursor = fetchRender.Execute ();
        const auto *render = static_cast<const Emergence::Celerity::Render2dSingleton *> (*renderCursor);

        const auto uiCursor = fetchUI.Execute ();
        const auto *ui = static_cast<const Emergence::Celerity::UISingleton *> (*uiCursor);

        auto mainMenuCursor = modifyMainMenu.Execute ();
        auto *mainMenu = static_cast<MainMenuSingleton *> (*mainMenuCursor);

        auto transformCursor = insertTransform.Execute ();
        auto cameraCursor = insertCamera.Execute ();
        auto spriteCursor = insertSprite.Execute ();
        auto inputSubscriptionCursor = insertInputSubscription.Execute ();

        auto *inputSubscriptionTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        inputSubscriptionTransform->SetObjectId (world->GenerateId ());

        auto *inputSubscription =
            static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++inputSubscriptionCursor);
        inputSubscription->objectId = inputSubscriptionTransform->GetObjectId ();
        inputSubscription->group = MainMenuInputActions::GetMainMenuUIGroup ();
        mainMenu->uiListenerObjectId = inputSubscription->objectId;

        auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        cameraTransform->SetObjectId (world->GenerateId ());
        cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

        auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
        camera->objectId = cameraTransform->GetObjectId ();
        camera->halfOrthographicSize = 3.75f;
        camera->visibilityMask = static_cast<uint64_t> (VisibilityMask::GAME_SCENE);

        auto viewportCursor = insertViewport.Execute ();
        auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

        worldViewport->name = "MainMenuScene"_us;
        worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
        worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
        worldViewport->clearColor = 0x000000FF;
        worldViewport->sortIndex = 0u;

        auto worldPassCursor = insertWorldPass.Execute ();
        auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);

        worldPass->name = worldViewport->name;
        worldPass->cameraObjectId = camera->objectId;

        for (size_t index = 0u; index < 3u; ++index)
        {
            auto *spriteTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
            spriteTransform->SetObjectId (world->GenerateId ());
            spriteTransform->SetVisualLocalTransform (
                {{-10.0f + 10.0f * static_cast<float> (index), 0.0f}, 0.0f, {1.0f, 1.0f}});

            auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (++spriteCursor);
            sprite->objectId = spriteTransform->GetObjectId ();
            sprite->spriteId = render->GenerateSprite2dId ();
            sprite->visibilityMask = static_cast<uint64_t> (VisibilityMask::GAME_SCENE);

            sprite->materialInstanceId = "MenuBackgroundTile"_us;
            sprite->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
            sprite->halfSize = {5.0f, 3.75f};
            sprite->layer = 0u;
        }

        auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
        uiViewport->name = "MainMenuUI"_us;
        uiViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
        uiViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
        uiViewport->clearColor = 0x00000000;
        uiViewport->sortIndex = 1u;

        auto uiPassCursor = insertUIPass.Execute ();
        auto *uiPass = static_cast<Emergence::Celerity::UIRenderPass *> (++uiPassCursor);
        uiPass->name = uiViewport->name;
        uiPass->defaultStyleId = "Default"_us;

        auto uiNodeCursor = insertUINode.Execute ();
        auto *mainWindowNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        mainWindowNode->nodeId = ui->GenerateNodeId ();
        mainWindowNode->sortIndex = 0u;

        auto windowControlCursor = insertWindowControl.Execute ();
        auto *mainWindow = static_cast<Emergence::Celerity::WindowControl *> (++windowControlCursor);
        mainWindow->nodeId = mainWindowNode->nodeId;
        mainWindow->viewportName = uiPass->name;
        mainWindow->title = "Main Menu";
        mainWindow->closable = false;
        mainWindow->minimizable = false;
        mainWindow->resizable = false;
        mainWindow->movable = false;
        mainWindow->hasTitleBar = false;
        mainWindow->width = 215u;
        mainWindow->height = 175u;
        mainWindow->anchor = {0.5f, 0.5f};
        mainWindow->pivot = {0.5f, 0.5f};

        auto *startTutorialButtonNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        startTutorialButtonNode->nodeId = ui->GenerateNodeId ();
        startTutorialButtonNode->parentId = mainWindow->nodeId;
        startTutorialButtonNode->sortIndex = 0u;

        auto buttonControlCursor = insertButtonControl.Execute ();
        auto *startTutorialButton = static_cast<Emergence::Celerity::ButtonControl *> (++buttonControlCursor);
        startTutorialButton->nodeId = startTutorialButtonNode->nodeId;
        startTutorialButton->label = "Start tutorial";

        startTutorialButton->width = 200u;
        startTutorialButton->height = 50u;
        startTutorialButton->onClickAction = MainMenuInputActions::GetStartTutorialAction ();
        startTutorialButton->onClickActionDispatch = Emergence::Celerity::InputActionDispatchType::NORMAL;

        auto *startCampaignButtonNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        startCampaignButtonNode->nodeId = ui->GenerateNodeId ();
        startCampaignButtonNode->parentId = mainWindow->nodeId;
        startCampaignButtonNode->sortIndex = 1u;

        auto *startCampaignButton = static_cast<Emergence::Celerity::ButtonControl *> (++buttonControlCursor);
        startCampaignButton->nodeId = startCampaignButtonNode->nodeId;
        startCampaignButton->label = "Start campaign";

        startCampaignButton->width = startTutorialButton->width;
        startCampaignButton->height = startTutorialButton->height;
        startCampaignButton->onClickAction = MainMenuInputActions::GetStartCampaignAction ();
        startCampaignButton->onClickActionDispatch = Emergence::Celerity::InputActionDispatchType::NORMAL;

        auto *quitButtonNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        quitButtonNode->nodeId = ui->GenerateNodeId ();
        quitButtonNode->parentId = mainWindow->nodeId;
        quitButtonNode->sortIndex = 2u;

        auto *quitButton = static_cast<Emergence::Celerity::ButtonControl *> (++buttonControlCursor);
        quitButton->nodeId = quitButtonNode->nodeId;
        quitButton->label = "Quit";

        quitButton->width = startTutorialButton->width;
        quitButton->height = startTutorialButton->height;
        quitButton->onClickAction = MainMenuInputActions::GetQuitAction ();
        quitButton->onClickActionDispatch = Emergence::Celerity::InputActionDispatchType::NORMAL;

        auto fontPropertyCursor = insertFontProperty.Execute ();
        auto *fontProperty = static_cast<Emergence::Celerity::UIStyleFontProperty *> (++fontPropertyCursor);
        fontProperty->styleId = uiPass->defaultStyleId;
        fontProperty->fontId = Emergence::Memory::UniqueString {
            EMERGENCE_BUILD_STRING ("DroidSans.ttf", Emergence::Celerity::FONT_SIZE_SEPARATOR, "20")};

        mainMenuLoading->mainMenuInitialized = true;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"MainMenuInitializer"}).SetExecutor<Initializer> ();
}
} // namespace MainMenuInitialization
