#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Application/Application.hpp>

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/Events.hpp>
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

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Yaml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

#include <SyntaxSugar/Time.hpp>

using namespace Emergence::Memory::Literals;

// Silence error about absent allocation group for asset paths array.
// TODO: Rework that later while refactoring demo.
EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Emergence::Memory::UniqueString)

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Emergence::Memory::UniqueString>::Get () noexcept
{
    return Profiler::AllocationGroup {Profiler::AllocationGroup::Top (), "UnhandledUniqueStringRefs"_us};
}
} // namespace Emergence::Memory

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

static Emergence::Celerity::AssetReferenceBindingList GetAssetReferenceBindingList ()
{
    Emergence::Celerity::AssetReferenceBindingList binding {Emergence::Celerity::GetAssetBindingAllocationGroup ()};
    Emergence::Celerity::GetRender2dAssetUsage (binding);
    Emergence::Celerity::GetRenderFoundationAssetUsage (binding);
    Emergence::Celerity::GetUIAssetUsage (binding);
    return binding;
}

static Emergence::Celerity::KeyCodeMapping GetSDLKeyCodeMapping ()
{
    return {
        SDLK_RETURN,
        SDLK_ESCAPE,
        SDLK_BACKSPACE,
        SDLK_TAB,
        SDLK_SPACE,
        SDLK_EXCLAIM,
        SDLK_QUOTEDBL,
        SDLK_HASH,
        SDLK_PERCENT,
        SDLK_DOLLAR,
        SDLK_AMPERSAND,
        SDLK_QUOTE,
        SDLK_LEFTPAREN,
        SDLK_RIGHTPAREN,
        SDLK_ASTERISK,
        SDLK_PLUS,
        SDLK_COMMA,
        SDLK_MINUS,
        SDLK_PERIOD,
        SDLK_SLASH,
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_COLON,
        SDLK_SEMICOLON,
        SDLK_LESS,
        SDLK_EQUALS,
        SDLK_GREATER,
        SDLK_QUESTION,
        SDLK_AT,

        SDLK_LEFTBRACKET,
        SDLK_BACKSLASH,
        SDLK_RIGHTBRACKET,
        SDLK_CARET,
        SDLK_UNDERSCORE,
        SDLK_BACKQUOTE,
        SDLK_a,
        SDLK_b,
        SDLK_c,
        SDLK_d,
        SDLK_e,
        SDLK_f,
        SDLK_g,
        SDLK_h,
        SDLK_i,
        SDLK_j,
        SDLK_k,
        SDLK_l,
        SDLK_m,
        SDLK_n,
        SDLK_o,
        SDLK_p,
        SDLK_q,
        SDLK_r,
        SDLK_s,
        SDLK_t,
        SDLK_u,
        SDLK_v,
        SDLK_w,
        SDLK_x,
        SDLK_y,
        SDLK_z,

        SDLK_CAPSLOCK,

        SDLK_F1,
        SDLK_F2,
        SDLK_F3,
        SDLK_F4,
        SDLK_F5,
        SDLK_F6,
        SDLK_F7,
        SDLK_F8,
        SDLK_F9,
        SDLK_F10,
        SDLK_F11,
        SDLK_F12,

        SDLK_PRINTSCREEN,
        SDLK_SCROLLLOCK,
        SDLK_PAUSE,
        SDLK_INSERT,
        SDLK_HOME,
        SDLK_PAGEUP,
        SDLK_DELETE,
        SDLK_END,
        SDLK_PAGEDOWN,
        SDLK_RIGHT,
        SDLK_LEFT,
        SDLK_DOWN,
        SDLK_UP,

        SDLK_NUMLOCKCLEAR,
        SDLK_KP_DIVIDE,
        SDLK_KP_MULTIPLY,
        SDLK_KP_MINUS,
        SDLK_KP_PLUS,
        SDLK_KP_ENTER,
        SDLK_KP_1,
        SDLK_KP_2,
        SDLK_KP_3,
        SDLK_KP_4,
        SDLK_KP_5,
        SDLK_KP_6,
        SDLK_KP_7,
        SDLK_KP_8,
        SDLK_KP_9,
        SDLK_KP_0,
        SDLK_KP_PERIOD,

        SDLK_APPLICATION,
        SDLK_POWER,
        SDLK_KP_EQUALS,
        SDLK_F13,
        SDLK_F14,
        SDLK_F15,
        SDLK_F16,
        SDLK_F17,
        SDLK_F18,
        SDLK_F19,
        SDLK_F20,
        SDLK_F21,
        SDLK_F22,
        SDLK_F23,
        SDLK_F24,
        SDLK_EXECUTE,
        SDLK_HELP,
        SDLK_MENU,
        SDLK_SELECT,
        SDLK_STOP,
        SDLK_AGAIN,
        SDLK_UNDO,
        SDLK_CUT,
        SDLK_COPY,
        SDLK_PASTE,
        SDLK_FIND,
        SDLK_MUTE,
        SDLK_VOLUMEUP,
        SDLK_VOLUMEDOWN,
        SDLK_KP_COMMA,
        SDLK_KP_EQUALSAS400,

        SDLK_ALTERASE,
        SDLK_SYSREQ,
        SDLK_CANCEL,
        SDLK_CLEAR,
        SDLK_PRIOR,
        SDLK_RETURN2,
        SDLK_SEPARATOR,
        SDLK_OUT,
        SDLK_OPER,
        SDLK_CLEARAGAIN,
        SDLK_CRSEL,
        SDLK_EXSEL,

        SDLK_KP_00,
        SDLK_KP_000,
        SDLK_THOUSANDSSEPARATOR,
        SDLK_DECIMALSEPARATOR,
        SDLK_CURRENCYUNIT,
        SDLK_CURRENCYSUBUNIT,
        SDLK_KP_LEFTPAREN,
        SDLK_KP_RIGHTPAREN,
        SDLK_KP_LEFTBRACE,
        SDLK_KP_RIGHTBRACE,
        SDLK_KP_TAB,
        SDLK_KP_BACKSPACE,
        SDLK_KP_A,
        SDLK_KP_B,
        SDLK_KP_C,
        SDLK_KP_D,
        SDLK_KP_E,
        SDLK_KP_F,
        SDLK_KP_XOR,
        SDLK_KP_POWER,
        SDLK_KP_PERCENT,
        SDLK_KP_LESS,
        SDLK_KP_GREATER,
        SDLK_KP_AMPERSAND,
        SDLK_KP_DBLAMPERSAND,
        SDLK_KP_VERTICALBAR,
        SDLK_KP_DBLVERTICALBAR,
        SDLK_KP_COLON,
        SDLK_KP_HASH,
        SDLK_KP_SPACE,
        SDLK_KP_AT,
        SDLK_KP_EXCLAM,
        SDLK_KP_MEMSTORE,
        SDLK_KP_MEMRECALL,
        SDLK_KP_MEMCLEAR,
        SDLK_KP_MEMADD,
        SDLK_KP_MEMSUBTRACT,
        SDLK_KP_MEMMULTIPLY,
        SDLK_KP_MEMDIVIDE,
        SDLK_KP_PLUSMINUS,
        SDLK_KP_CLEAR,
        SDLK_KP_CLEARENTRY,
        SDLK_KP_BINARY,
        SDLK_KP_OCTAL,
        SDLK_KP_DECIMAL,
        SDLK_KP_HEXADECIMAL,

        SDLK_LCTRL,
        SDLK_LSHIFT,
        SDLK_LALT,
        SDLK_LGUI,
        SDLK_RCTRL,
        SDLK_RSHIFT,
        SDLK_RALT,
        SDLK_RGUI,

        SDLK_MODE,

        SDLK_AUDIONEXT,
        SDLK_AUDIOPREV,
        SDLK_AUDIOSTOP,
        SDLK_AUDIOPLAY,
        SDLK_AUDIOMUTE,
        SDLK_MEDIASELECT,
        SDLK_WWW,
        SDLK_MAIL,
        SDLK_CALCULATOR,
        SDLK_COMPUTER,
        SDLK_AC_SEARCH,
        SDLK_AC_HOME,
        SDLK_AC_BACK,
        SDLK_AC_FORWARD,
        SDLK_AC_STOP,
        SDLK_AC_REFRESH,
        SDLK_AC_BOOKMARKS,

        SDLK_BRIGHTNESSDOWN,
        SDLK_BRIGHTNESSUP,
        SDLK_DISPLAYSWITCH,
        SDLK_KBDILLUMTOGGLE,
        SDLK_KBDILLUMDOWN,
        SDLK_KBDILLUMUP,
        SDLK_EJECT,
        SDLK_SLEEP,
        SDLK_APP1,
        SDLK_APP2,

        SDLK_AUDIOREWIND,
        SDLK_AUDIOFASTFORWARD,
    };
}

static Emergence::Memory::Profiler::EventObserver StartMemoryRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

WindowBackend::~WindowBackend () noexcept
{
    if (window)
    {
        Emergence::Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}

void WindowBackend::Init (const Settings &_settings) noexcept
{
    uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

    if (_settings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow ("Platformed2dDemo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               static_cast<int> (_settings.width), static_cast<int> (_settings.height),
                               static_cast<SDL_WindowFlags> (windowFlags));

    SDL_SysWMinfo windowsManagerInfo;
    SDL_VERSION (&windowsManagerInfo.version);
    SDL_GetWindowWMInfo (window, &windowsManagerInfo);

#if SDL_VIDEO_DRIVER_X11
    void *nativeDisplayType = windowsManagerInfo.info.x11.display;
    void *nativeWindowHandle = (void *) (uintptr_t) windowsManagerInfo.info.x11.window;
#elif SDL_VIDEO_DRIVER_COCOA
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.cocoa.window;
#elif SDL_VIDEO_DRIVER_WINDOWS
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.win.window;
#elif SDL_VIDEO_DRIVER_VIVANTE
    void *nativeDisplayType = windowsManagerInfo.info.vivante.display;
    void *nativeWindowHandle = windowsManagerInfo.info.vivante.window;
#endif

    Emergence::Render::Backend::Config config;
    config.width = _settings.width;
    config.height = _settings.height;
    config.vsync = _settings.vsync;
    Emergence::Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

SDL_Window *WindowBackend::GetWindow () const noexcept
{
    return window;
}

Application::Application () noexcept
    : memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartMemoryRecording (memoryEventSerializer, memoryEventOutput))
{
    Emergence::SetIsAssertInteractive (true);
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        sdlTicksAfterInit = SDL_GetTicks64 ();
        sdlInitTimeNs = Emergence::Time::NanosecondsSinceStartup ();
        Emergence::ReportCriticalError ("SDL initialization", __FILE__, __LINE__);
    }
}

void Application::Run () noexcept
{
    LoadSettings ();
    InitWindow ();
    InitWorld ();
    EventLoop ();
}

void Application::LoadSettings () noexcept
{
    std::filesystem::path settingsPath = "Settings.yaml";
    if (std::filesystem::exists (settingsPath))
    {
        EMERGENCE_LOG (INFO, "Application: Loading settings...");
        std::ifstream input {settingsPath};
        Emergence::Serialization::FieldNameLookupCache cache {Settings::Reflect ().mapping};

        if (!Emergence::Serialization::Yaml::DeserializeObject (input, &settings, cache))
        {
            EMERGENCE_LOG (INFO, "Application: Failed to load settings, falling back to default.");
            settings = {};
        }
    }
    else
    {
        EMERGENCE_LOG (INFO, "Application: Saving default settings...");
        std::ofstream output {settingsPath};
        Emergence::Serialization::Yaml::SerializeObject (output, &settings, Settings::Reflect ().mapping);
    }
}

void Application::InitWindow () noexcept
{
    EMERGENCE_LOG (INFO, "Application: Initializing window...");
    windowBackend.Init (settings);
}

void Application::InitWorld () noexcept
{
    Emergence::Celerity::AssetReferenceBindingList assetReferenceBindingList = GetAssetReferenceBindingList ();
    Emergence::Celerity::AssetReferenceBindingEventMap assetReferenceBindingEventMap;

    {
        Emergence::Celerity::EventRegistrar registrar {&world};
        Emergence::Celerity::RegisterAssemblyEvents (registrar);
        assetReferenceBindingEventMap = Emergence::Celerity::RegisterAssetEvents (registrar, assetReferenceBindingList);
        Emergence::Celerity::RegisterRender2dEvents (registrar);
        Emergence::Celerity::RegisterRenderFoundationEvents (registrar);
        Emergence::Celerity::RegisterTransform2dEvents (registrar);
        Emergence::Celerity::RegisterTransformCommonEvents (registrar);
        Emergence::Celerity::RegisterUIEvents (registrar);
    }

    Emergence::Celerity::PipelineBuilder pipelineBuilder {world.GetRootView ()};
    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    Emergence::Celerity::Input::AddToFixedUpdate (pipelineBuilder);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToFixedUpdate (pipelineBuilder);
    const bool fixedPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (fixedPipelineRegistered);

    constexpr uint64_t MAX_LOADING_TIME_NS = 10000000;
    static const Emergence::Memory::UniqueString gameMaterialInstancesPath {"../GameResources/MaterialInstances"};
    static const Emergence::Memory::UniqueString gameMaterialsPath {"../GameResources/Materials"};
    static const Emergence::Memory::UniqueString engineMaterialsPath {"../Render2dResources/Materials"};
    static const Emergence::Memory::UniqueString uiMaterialsPath {"../ImGUIResources/Materials"};
    static const Emergence::Memory::UniqueString gameShadersPath {"../GameResources/Shaders"};
    static const Emergence::Memory::UniqueString engineShadersPath {"../Render2dResources/Shaders"};
    static const Emergence::Memory::UniqueString uiShadersPath {"../ImGUIResources/Shaders"};
    static const Emergence::Memory::UniqueString gameTexturesPath {"../GameResources/Textures"};
    static const Emergence::Memory::UniqueString gameFontsPath {"../GameResources/Fonts"};
    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Emergence::Celerity::AssetManagement::AddToNormalUpdate (pipelineBuilder, assetReferenceBindingList,
                                                             assetReferenceBindingEventMap);
    Emergence::Celerity::FontManagement::AddToNormalUpdate (pipelineBuilder, {gameFontsPath}, MAX_LOADING_TIME_NS,
                                                            assetReferenceBindingEventMap);
    Emergence::Celerity::Input::AddToNormalUpdate (pipelineBuilder, &inputAccumulator);
    Emergence::Celerity::MaterialInstanceManagement::AddToNormalUpdate (
        pipelineBuilder, {gameMaterialInstancesPath}, MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::MaterialManagement::AddToNormalUpdate (
        pipelineBuilder, {gameMaterialsPath, engineMaterialsPath, uiMaterialsPath},
        {gameShadersPath, engineShadersPath, uiShadersPath}, MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (pipelineBuilder, worldBox);
    Emergence::Celerity::TextureManagement::AddToNormalUpdate (pipelineBuilder, {gameTexturesPath}, MAX_LOADING_TIME_NS,
                                                               assetReferenceBindingEventMap);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::UI::AddToNormalUpdate (pipelineBuilder, &inputAccumulator, GetSDLKeyCodeMapping ());
    pipelineBuilder.AddTask ("DemoScenarioExecutor"_us).SetExecutor<DemoScenarioExecutor> ();
    const bool normalPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (normalPipelineRegistered);
}

void Application::EventLoop () noexcept
{
    EMERGENCE_LOG (INFO, "Application: Starting event loop...");
    bool running = true;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent (&event))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID (windowBackend.GetWindow ())))
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                Emergence::Celerity::KeyboardEvent inputEvent {
                    static_cast<Emergence::Celerity::KeyCode> (event.key.keysym.sym),
                    static_cast<Emergence::Celerity::ScanCode> (event.key.keysym.scancode),
                    static_cast<Emergence::Celerity::QualifiersMask> (event.key.keysym.mod),
                    event.type == SDL_KEYDOWN ? Emergence::Celerity::KeyState::DOWN : Emergence::Celerity::KeyState::UP,
                };

                inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                Emergence::Celerity::MouseButton button = Emergence::Celerity::MouseButton::LEFT;
                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    button = Emergence::Celerity::MouseButton::LEFT;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = Emergence::Celerity::MouseButton::MIDDLE;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = Emergence::Celerity::MouseButton::RIGHT;
                    break;
                case SDL_BUTTON_X1:
                    button = Emergence::Celerity::MouseButton::X1;
                    break;
                case SDL_BUTTON_X2:
                    button = Emergence::Celerity::MouseButton::X2;
                    break;
                }

                Emergence::Celerity::MouseButtonEvent inputEvent {
                    event.button.x,
                    event.button.y,
                    button,
                    event.type == SDL_MOUSEBUTTONDOWN ? Emergence::Celerity::KeyState::DOWN :
                                                        Emergence::Celerity::KeyState::UP,
                    event.button.clicks,
                };

                inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                Emergence::Celerity::MouseMotionEvent inputEvent {
                    event.motion.x - event.motion.xrel,
                    event.motion.y - event.motion.yrel,
                    event.motion.x,
                    event.motion.y,
                };

                inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {
                Emergence::Celerity::MouseWheelEvent inputEvent {
                    event.wheel.preciseX,
                    event.wheel.preciseY,
                };

                inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                Emergence::Celerity::TextInputEvent inputEvent;
                static_assert (sizeof (inputEvent.utf8Value) >= sizeof (event.text.text));
                strcpy (inputEvent.utf8Value.data (), event.text.text);
                inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
        }

        world.Update ();
        inputAccumulator.Clear ();

        while (const Emergence::Memory::Profiler::Event *memoryEvent = memoryEventObserver.NextEvent ())
        {
            memoryEventSerializer.SerializeEvent (*memoryEvent);
        }
    }
}

uint64_t Application::SDLTicksToTime (uint64_t _ticks) const noexcept
{
    return (_ticks - sdlTicksAfterInit) * 1000000u + sdlInitTimeNs;
}
