#include <Application/KeyCodeMapping.hpp>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/InputControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/UI.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/UI/UISingleton.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Math/Constants.hpp>

#include <Modules/Demo.hpp>

#include <Render/Backend/Configuration.hpp>

using namespace Emergence::Memory::Literals;

struct DemoScenarioExecutor final : public Emergence::Celerity::TaskExecutorBase<DemoScenarioExecutor>
{
public:
    DemoScenarioExecutor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTimeSingleton;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchUI;
    Emergence::Celerity::ModifySingletonQuery modifyRender;

    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertSprite;

    Emergence::Celerity::InsertLongTermQuery insertUINode;
    Emergence::Celerity::InsertLongTermQuery insertWindowControl;
    Emergence::Celerity::InsertLongTermQuery insertButtonControl;
    Emergence::Celerity::InsertLongTermQuery insertCheckboxControl;
    Emergence::Celerity::InsertLongTermQuery insertContainerControl;
    Emergence::Celerity::InsertLongTermQuery insertImageControl;
    Emergence::Celerity::InsertLongTermQuery insertInputControl;
    Emergence::Celerity::InsertLongTermQuery insertLabelControl;
    Emergence::Celerity::InsertLongTermQuery insertFontProperty;

    Emergence::Celerity::EditValueQuery editUniformVector4fByAssetIdAndName;

    bool initializationDone = false;
};

DemoScenarioExecutor::DemoScenarioExecutor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTimeSingleton (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchUI (FETCH_SINGLETON (Emergence::Celerity::UISingleton)),
      modifyRender (MODIFY_SINGLETON (Emergence::Celerity::Render2dSingleton)),

      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertSprite (INSERT_LONG_TERM (Emergence::Celerity::Sprite2dComponent)),

      insertUINode (INSERT_LONG_TERM (Emergence::Celerity::UINode)),
      insertWindowControl (INSERT_LONG_TERM (Emergence::Celerity::WindowControl)),
      insertButtonControl (INSERT_LONG_TERM (Emergence::Celerity::ButtonControl)),
      insertCheckboxControl (INSERT_LONG_TERM (Emergence::Celerity::CheckboxControl)),
      insertContainerControl (INSERT_LONG_TERM (Emergence::Celerity::ContainerControl)),
      insertImageControl (INSERT_LONG_TERM (Emergence::Celerity::ImageControl)),
      insertInputControl (INSERT_LONG_TERM (Emergence::Celerity::InputControl)),
      insertLabelControl (INSERT_LONG_TERM (Emergence::Celerity::LabelControl)),
      insertFontProperty (INSERT_LONG_TERM (Emergence::Celerity::UIStyleFontProperty)),

      editUniformVector4fByAssetIdAndName (
          EDIT_VALUE_2F (Emergence::Celerity::UniformVector4fValue, assetId, uniformName))
{
    _constructor.DependOn (Emergence::Celerity::AssetManagement::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::TransformVisualSync::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (Emergence::Celerity::UI::Checkpoint::HIERARCHY_CLEANUP_STARTED);
}

void DemoScenarioExecutor::Execute () noexcept
{
    if (!initializationDone)
    {
        auto worldCursor = fetchWorld.Execute ();
        const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

        const auto uiCursor = fetchUI.Execute ();
        const auto *ui = static_cast<const Emergence::Celerity::UISingleton *> (*uiCursor);

        auto renderCursor = modifyRender.Execute ();
        auto *render = static_cast<Emergence::Celerity::Render2dSingleton *> (*renderCursor);

        auto transformCursor = insertTransform.Execute ();
        auto cameraCursor = insertCamera.Execute ();
        auto spriteCursor = insertSprite.Execute ();

        auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        cameraTransform->SetObjectId (world->GenerateId ());
        cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

        auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
        camera->objectId = cameraTransform->GetObjectId ();
        camera->halfOrthographicSize = 5.0f;

        auto viewportCursor = insertViewport.Execute ();
        auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

        worldViewport->name = "GameWorld"_us;
        worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
        worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
        worldViewport->clearColor = 0xAAAAFFFF;
        worldViewport->sortIndex = 0u;

        auto worldPassCursor = insertWorldPass.Execute ();
        auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);

        worldPass->name = worldViewport->name;
        worldPass->cameraObjectId = camera->objectId;

        for (std::size_t index = 0u; index < 20u; ++index)
        {
            auto *spriteTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
            spriteTransform->SetObjectId (world->GenerateId ());
            spriteTransform->SetVisualLocalTransform (
                {{-10.0f + static_cast<float> (index), -1.5f}, 0.0f, {1.0f, 1.0f}});

            auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (++spriteCursor);
            sprite->objectId = spriteTransform->GetObjectId ();
            sprite->spriteId = render->GenerateSprite2dId ();

            sprite->materialInstanceId = "Earth"_us;
            sprite->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
            sprite->halfSize = {0.5f, 0.5f};
            sprite->layer = 2u;
        }

        auto *spriteTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        spriteTransform->SetObjectId (world->GenerateId ());

        auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (++spriteCursor);
        sprite->objectId = spriteTransform->GetObjectId ();
        sprite->spriteId = render->GenerateSprite2dId ();

        sprite->materialInstanceId = "CrateLoading"_us;
        sprite->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
        sprite->halfSize = {3.0f, 3.0f};
        sprite->layer = 0u;

        auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
        uiViewport->name = "UI"_us;
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
        mainWindow->title = "Main Window";
        mainWindow->width = 300;
        mainWindow->height = 200;
        mainWindow->anchor = {0.5f, 0.5f};
        mainWindow->pivot = {0.5f, 0.5f};
        mainWindow->closable = false;

        auto *firstRowNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        firstRowNode->nodeId = ui->GenerateNodeId ();
        firstRowNode->parentId = mainWindowNode->nodeId;
        firstRowNode->sortIndex = 0u;

        auto containerControlCursor = insertContainerControl.Execute ();
        auto *firstRowContainer = static_cast<Emergence::Celerity::ContainerControl *> (++containerControlCursor);
        firstRowContainer->nodeId = firstRowNode->nodeId;
        firstRowContainer->type = Emergence::Celerity::ContainerControlType::COLLAPSING_PANEL;
        firstRowContainer->label = "First row";
        firstRowContainer->layout = Emergence::Celerity::ContainerControlLayout::HORIZONTAL;

        auto *firstRowFirstButtonNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        firstRowFirstButtonNode->nodeId = ui->GenerateNodeId ();
        firstRowFirstButtonNode->parentId = firstRowNode->nodeId;
        firstRowFirstButtonNode->sortIndex = 0u;

        auto buttonControlCursor = insertButtonControl.Execute ();
        auto *firstRowFirstButton = static_cast<Emergence::Celerity::ButtonControl *> (++buttonControlCursor);
        firstRowFirstButton->nodeId = firstRowFirstButtonNode->nodeId;
        firstRowFirstButton->label = "Hello, world!";

        auto *firstRowSecondButtonNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        firstRowSecondButtonNode->nodeId = ui->GenerateNodeId ();
        firstRowSecondButtonNode->parentId = firstRowNode->nodeId;
        firstRowSecondButtonNode->sortIndex = 1u;

        auto *firstRowSecondButton = static_cast<Emergence::Celerity::ButtonControl *> (++buttonControlCursor);
        firstRowSecondButton->nodeId = firstRowSecondButtonNode->nodeId;
        firstRowSecondButton->label = "Click me!";

        auto *secondRowNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        secondRowNode->nodeId = ui->GenerateNodeId ();
        secondRowNode->parentId = mainWindowNode->nodeId;
        secondRowNode->sortIndex = 1u;

        auto *secondRowContainer = static_cast<Emergence::Celerity::ContainerControl *> (++containerControlCursor);
        secondRowContainer->nodeId = secondRowNode->nodeId;
        secondRowContainer->type = Emergence::Celerity::ContainerControlType::PANEL;
        secondRowContainer->border = true;
        secondRowContainer->layout = Emergence::Celerity::ContainerControlLayout::HORIZONTAL;
        secondRowContainer->width = 250u;
        secondRowContainer->height = 50u;

        auto *secondRowLabelNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        secondRowLabelNode->nodeId = ui->GenerateNodeId ();
        secondRowLabelNode->parentId = secondRowNode->nodeId;
        secondRowLabelNode->sortIndex = 100u;

        auto labelControlCursor = insertLabelControl.Execute ();
        auto *secondRowLabel = static_cast<Emergence::Celerity::LabelControl *> (++labelControlCursor);
        secondRowLabel->nodeId = secondRowLabelNode->nodeId;
        secondRowLabel->label = "Right!";

        auto *secondRowCheckboxNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        secondRowCheckboxNode->nodeId = ui->GenerateNodeId ();
        secondRowCheckboxNode->parentId = secondRowNode->nodeId;
        secondRowCheckboxNode->sortIndex = 0u;

        auto checkboxControlCursor = insertCheckboxControl.Execute ();
        auto *secondRowCheckbox = static_cast<Emergence::Celerity::CheckboxControl *> (++checkboxControlCursor);
        secondRowCheckbox->nodeId = secondRowCheckboxNode->nodeId;
        secondRowCheckbox->label = "Some checkbox";

        auto *secondRowImageNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        secondRowImageNode->nodeId = ui->GenerateNodeId ();
        secondRowImageNode->parentId = secondRowNode->nodeId;
        secondRowImageNode->sortIndex = 1u;

        auto imageControlCursor = insertImageControl.Execute ();
        auto *secondRowImage = static_cast<Emergence::Celerity::ImageControl *> (++imageControlCursor);
        secondRowImage->nodeId = secondRowImageNode->nodeId;
        secondRowImage->width = 40u;
        secondRowImage->height = 40u;
        secondRowImage->textureId = Emergence::Memory::UniqueString {"Crate.png"};
        secondRowImage->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};

        auto *integerInputNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        integerInputNode->nodeId = ui->GenerateNodeId ();
        integerInputNode->parentId = mainWindowNode->nodeId;
        integerInputNode->sortIndex = 2u;

        auto inputControlCursor = insertInputControl.Execute ();
        auto *integerInput = static_cast<Emergence::Celerity::InputControl *> (++inputControlCursor);
        integerInput->nodeId = integerInputNode->nodeId;
        integerInput->type = Emergence::Celerity::InputControlType::INT;
        integerInput->label = "Input int";

        auto *floatInputNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        floatInputNode->nodeId = ui->GenerateNodeId ();
        floatInputNode->parentId = mainWindowNode->nodeId;
        floatInputNode->sortIndex = 3u;

        auto *floatInput = static_cast<Emergence::Celerity::InputControl *> (++inputControlCursor);
        floatInput->nodeId = floatInputNode->nodeId;
        floatInput->type = Emergence::Celerity::InputControlType::FLOAT;
        floatInput->label = "Input float";

        auto *textInputNode = static_cast<Emergence::Celerity::UINode *> (++uiNodeCursor);
        textInputNode->nodeId = ui->GenerateNodeId ();
        textInputNode->parentId = mainWindowNode->nodeId;
        textInputNode->sortIndex = 4u;

        auto *textInput = static_cast<Emergence::Celerity::InputControl *> (++inputControlCursor);
        textInput->nodeId = textInputNode->nodeId;
        textInput->type = Emergence::Celerity::InputControlType::TEXT;
        textInput->label = "Input text";

        auto fontPropertyCursor = insertFontProperty.Execute ();
        auto *fontProperty = static_cast<Emergence::Celerity::UIStyleFontProperty *> (++fontPropertyCursor);
        fontProperty->styleId = uiPass->defaultStyleId;
        fontProperty->fontId = Emergence::Memory::UniqueString {
            EMERGENCE_BUILD_STRING ("DroidSans.ttf", Emergence::Celerity::FONT_SIZE_SEPARATOR, "14")};

        initializationDone = true;
    }

    auto timeCursor = fetchTimeSingleton.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    struct
    {
        Emergence::Memory::UniqueString assetId = "CrateLoading"_us;
        Emergence::Memory::UniqueString uniformName = "angle"_us;
    } query;

    if (auto uniformCursor = editUniformVector4fByAssetIdAndName.Execute (&query);
        auto *uniform = static_cast<Emergence::Celerity::UniformVector4fValue *> (*uniformCursor))
    {
        uniform->value.x = static_cast<float> (time->normalTimeNs % 1000000000u) * 1e-9f * 2.0f * Emergence::Math::PI;
    }
}

Emergence::Memory::UniqueString GetDemoModuleName () noexcept
{
    static Emergence::Memory::UniqueString name {"Demo"_us};
    return name;
}

Emergence::Celerity::WorldViewConfig GetDemoModuleViewConfig () noexcept
{
    static Emergence::Celerity::WorldViewConfig rootViewConfig = [] ()
    {
        Emergence::Celerity::WorldViewConfig config;
        return config;
    }();

    return rootViewConfig;
}

void DemoModuleInitializer (GameState &_gameState,
                            Emergence::Celerity::World & /*unused*/,
                            Emergence::Celerity::WorldView &_rootView) noexcept
{
    Emergence::Celerity::PipelineBuilder pipelineBuilder {&_rootView};
    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Emergence::Celerity::Input::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (pipelineBuilder);
    const bool fixedPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (fixedPipelineRegistered);

    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);

    // Mock asset management checkpoints.
    pipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::STARTED);
    pipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    pipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
    pipelineBuilder.AddCheckpoint (Emergence::Celerity::AssetManagement::Checkpoint::FINISHED);

    Emergence::Celerity::Input::AddToNormalUpdate (pipelineBuilder, _gameState.GetFrameInputAccumulator ());
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (pipelineBuilder, worldBox);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::UI::AddToNormalUpdate (pipelineBuilder, _gameState.GetFrameInputAccumulator (),
                                                GetKeyCodeMapping ());
    pipelineBuilder.AddTask ("DemoScenarioExecutor"_us).SetExecutor<DemoScenarioExecutor> ();
    const bool normalPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (normalPipelineRegistered);
}
