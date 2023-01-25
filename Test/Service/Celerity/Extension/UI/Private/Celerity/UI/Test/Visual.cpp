#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/Events.hpp>
#include <Celerity/UI/Test/ControlManagement.hpp>
#include <Celerity/UI/Test/ImplementationStrings.hpp>
#include <Celerity/UI/Test/SDLContextHolder.hpp>
#include <Celerity/UI/Test/Visual.hpp>
#include <Celerity/UI/UI.hpp>

#include <FileSystem/Test/Utility.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
bool VisualTestIncludeMarker () noexcept
{
    return true;
}

class ScreenshotTester final : public TaskExecutorBase<ScreenshotTester>
{
public:
    ScreenshotTester (TaskConstructor &_constructor, Container::String _passName, bool *_isFinishedOutput) noexcept;

    void Execute () noexcept;

private:
    Container::String GetOutputFileName () const noexcept;

    void CheckScreenshot () const noexcept;

    bool IsWaitingForDependencies () noexcept;

    void TakeScreenshot () const noexcept;

    FetchSingletonQuery fetchAssetManager;
    Container::String passName;
    bool firstUpdate = true;
    bool screenshotRequested = false;
    bool *isFinishedOutput = nullptr;
};

ScreenshotTester::ScreenshotTester (TaskConstructor &_constructor,
                                    Container::String _passName,
                                    bool *_isFinishedOutput) noexcept
    : fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),
      passName (std::move (_passName)),
      isFinishedOutput (_isFinishedOutput)
{
    _constructor.DependOn (RenderPipelineFoundation::Checkpoint::RENDER_FINISHED);
}

void ScreenshotTester::Execute () noexcept
{
    // Skip first update as asset dependencies aren't yet initialized.
    if (firstUpdate)
    {
        firstUpdate = false;
        return;
    }

    if (screenshotRequested)
    {
        CheckScreenshot ();
        *isFinishedOutput = true;
        return;
    }

    if (!IsWaitingForDependencies ())
    {
        TakeScreenshot ();
        screenshotRequested = true;
    }
    else
    {
        LOG ("Waiting for dependencies...");
    }
}

Container::String ScreenshotTester::GetOutputFileName () const noexcept
{
    return EMERGENCE_BUILD_STRING (GetUIBackendScreenshotPrefix (), "_", passName, ".png");
}

void ScreenshotTester::CheckScreenshot () const noexcept
{
    LOG ("Waiting for screenshot...");
    const Container::String screenshotFile = GetOutputFileName ();

    while (!std::filesystem::exists (screenshotFile))
    {
        std::this_thread::yield ();
    }

    LOG ("Giving renderer some time to write image...");
    std::this_thread::sleep_for (std::chrono::milliseconds {300u});
    LOG ("Starting image check.");

    FileSystem::Test::ExpectFilesEqual (
        EMERGENCE_BUILD_STRING ("UITestResources/Expectation/", GetUIBackendScreenshotPrefix (), "/", passName, ".png"),
        screenshotFile);
}

bool ScreenshotTester::IsWaitingForDependencies () noexcept
{
    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);
    return assetManager->assetsLeftToLoad > 0u;
}

void ScreenshotTester::TakeScreenshot () const noexcept
{
    LOG ("Taking screenshot...");
    Render::Backend::TakePngScreenshot (GetOutputFileName ().c_str ());
}

static void ExecuteScenario (Container::String _passName, Container::Vector<ControlManagement::Frame> _frames)
{
    using namespace Memory::Literals;

    World world {Emergence::Memory::UniqueString {"TestWorld"}, {{1.0f / 60.0f}}};
    FrameInputAccumulator inputAccumulator;
    AssetReferenceBindingList binding {GetAssetBindingAllocationGroup ()};
    AssetReferenceBindingEventMap assetReferenceBindingEventMap;
    GetUIAssetUsage (binding);

    {
        EventRegistrar registrar {&world};
        assetReferenceBindingEventMap = RegisterAssetEvents (registrar, binding);
        RegisterRenderFoundationEvents (registrar);
        RegisterUIEvents (registrar);
    }

    constexpr uint64_t MAX_LOADING_TIME_NS = 16000000;
    static const Emergence::Memory::UniqueString testMaterialsPath {"UITestResources/Materials"};
    static const Emergence::Memory::UniqueString engineMaterialsPath {GetUIBackendMaterialPath ()};
    static const Emergence::Memory::UniqueString engineShadersPath {GetUIBackendShaderPath ()};
    static const Emergence::Memory::UniqueString testTexturesPath {"UITestResources/Textures"};
    static const Emergence::Memory::UniqueString testFontsPath {"UITestResources/Fonts"};
    PipelineBuilder pipelineBuilder {world.GetRootView ()};

    pipelineBuilder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    AssetManagement::AddToNormalUpdate (pipelineBuilder, binding, assetReferenceBindingEventMap);
    ControlManagement::AddToNormalUpdate (pipelineBuilder, std::move (_frames));
    FontManagement::AddToNormalUpdate (pipelineBuilder, {testFontsPath}, MAX_LOADING_TIME_NS,
                                       assetReferenceBindingEventMap);
    Input::AddToNormalUpdate (pipelineBuilder, &inputAccumulator);
    MaterialManagement::AddToNormalUpdate (pipelineBuilder, {testMaterialsPath, engineMaterialsPath},
                                           {engineShadersPath}, MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    TextureManagement::AddToNormalUpdate (pipelineBuilder, {testTexturesPath}, MAX_LOADING_TIME_NS,
                                          assetReferenceBindingEventMap);
    UI::AddToNormalUpdate (pipelineBuilder, &inputAccumulator, {});

    bool testFinished = false;
    pipelineBuilder.AddTask ("ScreenshotTester"_us)
        .SetExecutor<ScreenshotTester> (std::move (_passName), &testFinished);

    // Mock transform hierarchy cleanup checkpoints.
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::STARTED);
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);
    REQUIRE (pipelineBuilder.End ());

    while (!testFinished)
    {
        ContextHolder::Frame ();
        world.Update ();
    }
}

} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test::ControlManagement::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

static CreateInput CreateIntInput (UniqueId _nodeId, UniqueId _parentId)
{
    CreateInput task;
    task.nodeId = _nodeId;
    task.parentId = _parentId;
    task.type = InputControlType::INT;
    task.label = "Integer input";
    task.intValue = 112u;
    return task;
}

static CreateInput CreateFloatInput (UniqueId _nodeId, UniqueId _parentId)
{
    CreateInput task;
    task.nodeId = _nodeId;
    task.parentId = _parentId;
    task.type = InputControlType::FLOAT;
    task.label = "Floating input";
    task.floatValue = 42.39f;
    return task;
}

static CreateInput CreateTextInput (UniqueId _nodeId, UniqueId _parentId)
{
    CreateInput task;
    task.nodeId = _nodeId;
    task.parentId = _parentId;
    task.type = InputControlType::TEXT;
    task.label = "Text input";
    strcpy (task.utf8TextValue.data (), "Hello, world!");
    return task;
}

BEGIN_SUITE (Visual)

TEST_CASE (CustomSkin)
{
    ExecuteScenario (
        "CustomSkin",
        {{
            CreateViewport {"UI"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF, "DefaultStyle"_us},

            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::TEXT, 0.86f, 0.93f, 0.87f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::DISABLED_TEXT, 0.52f, 0.55f, 0.53f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::WINDOW_BACKGROUND, 0.13f, 0.14f,
                                      0.17f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::CONTAINER_BACKGROUND, 0.15f, 0.16f,
                                      0.19f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::POPUP_BACKGROUND, 0.2f, 0.22f, 0.27f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::BORDER, 0.14f, 0.11f, 0.13f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::BORDER_SHADOW, 0.0f, 0.0f, 0.0f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::TITLE_BACKGROUND, 0.39f, 0.27f, 0.1f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::TITLE_BACKGROUND_ACTIVE, 0.69f,
                                      0.38f, 0.05f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::TITLE_BACKGROUND_COLLAPSED, 0.2f,
                                      0.22f, 0.27f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::SCROLLBAR_BACKGROUND, 0.2f, 0.22f,
                                      0.27f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::SCROLLBAR_GRAB, 0.73f, 0.49f, 0.13f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::SCROLLBAR_GRAB_HOVERED, 0.88f, 0.59f,
                                      0.64f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::SCROLLBAR_GRAB_ACTIVE, 0.55f, 0.38f,
                                      0.09f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::CHECK_MARK, 0.95f, 0.53f, 0.04f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::BUTTON, 0.76f, 0.41f, 0.11f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::BUTTON_HOVERED, 0.99f, 0.57f, 0.21f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::BUTTON_ACTIVE, 0.66f, 0.37f, 0.15f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::HEADER, 0.60f, 0.38f, 0.15f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::HEADER_HOVERED, 0.76f, 0.50f, 0.11f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::HEADER_ACTIVE, 0.58f, 0.36f, 0.09f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::RESIZE_GRIP, 0.8f, 0.48f, 0.14f,
                                      1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::RESIZE_GRIP_HOVERED, 0.95f, 0.51f,
                                      0.04f, 1.0f},
            CreateStyleColorProperty {"DefaultStyle"_us, UIStyleColorPropertyName::RESIZE_GRIP_ACTIVE, 0.61f, 0.39f,
                                      0.09f, 1.0f},

            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::ALPHA, 1.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::DISABLED_ALPHA, 0.4f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::WINDOW_ROUNDING, 4.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::WINDOW_BORDER_SIZE, 0.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::CONTAINER_ROUNDING, 0.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::CONTAINER_BORDER_SIZE, 1.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::POPUP_ROUNDING, 1.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::POPUP_BORDER_SIZE, 1.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::INDENT_SPACING, 6.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::SCROLLBAR_SIZE, 13.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::SCROLLBAR_ROUNDING, 16.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::GRAB_MIN_SIZE, 20.0f},
            CreateStyleFloatProperty {"DefaultStyle"_us, UIStyleFloatPropertyName::GRAB_ROUNDING, 4.0f},

            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::WINDOW_PADDING, 10.0f,
                                          10.0f},
            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::WINDOW_MIN_SIZE, 50.0f,
                                          50.0f},
            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::WINDOW_TITLE_ALIGN, 0.5f,
                                          0.5f},
            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::ITEM_SPACING, 6.0f, 6.0f},
            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::ITEM_INNER_SPACING, 3.0f,
                                          3.0f},
            CreateStyleFloatPairProperty {"DefaultStyle"_us, UIStyleFloatPairPropertyName::BUTTON_TEXT_ALIGN, 0.5f,
                                          0.5f},

            CreateStyleFontProperty {"DefaultStyle"_us, "DroidSans.ttf#14"_us},

            CreateStyleColorProperty {"OkButtonStyle"_us, UIStyleColorPropertyName::BUTTON, 0.0f, 0.5f, 0.0f, 1.0f},
            CreateStyleColorProperty {"OkButtonStyle"_us, UIStyleColorPropertyName::BUTTON_HOVERED, 0.0f, 0.9f, 0.0f,
                                      1.0f},
            CreateStyleColorProperty {"OkButtonStyle"_us, UIStyleColorPropertyName::BUTTON_ACTIVE, 0.0f, 0.3f, 0.0f,
                                      1.0f},

            CreateStyleColorProperty {"CancelButtonStyle"_us, UIStyleColorPropertyName::BUTTON, 0.5f, 0.0f, 0.0f, 1.0f},
            CreateStyleColorProperty {"CancelButtonStyle"_us, UIStyleColorPropertyName::BUTTON_HOVERED, 0.9f, 0.0f,
                                      0.0f, 1.0f},
            CreateStyleColorProperty {"CancelButtonStyle"_us, UIStyleColorPropertyName::BUTTON_ACTIVE, 0.3f, 0.0f, 0.0f,
                                      1.0f},

            CreateWindow {
                0u,           ""_us,        "UI"_us, "Left window!",
                true,         true,         true,    true,
                true,         false,        true,    ContainerControlLayout::VERTICAL,
                {0.0f, 0.0f}, {0.0f, 0.0f}, 0u,      0u,
                WIDTH / 2,    HEIGHT,       {},      {},
            },

            CreateLabel {1u, 0u, ""_us, "Example of dialog buttons:"},
            CreateContainer {2u, 0u, ""_us, ContainerControlType::PANEL, ContainerControlLayout::HORIZONTAL, 180u, 60u,
                             true, "", ""},
            CreateButton {3u, 2u, "OkButtonStyle"_us, "Ok", 50u, 30u, {}, InputActionDispatchType::NORMAL},
            CreateButton {4u, 2u, "CancelButtonStyle"_us, "Cancel", 50u, 30u, {}, InputActionDispatchType::NORMAL},
            CreateButton {5u, 2u, ""_us, "Help", 50u, 30u, {}, InputActionDispatchType::NORMAL},

            CreateContainer {6u, 0u, ""_us, ContainerControlType::COLLAPSING_PANEL, ContainerControlLayout::VERTICAL,
                             0u, 0u, false, "Collapsed one", ""},
            CreateLabel {7u, 6u, ""_us, "First unseen."},
            CreateLabel {8u, 6u, ""_us, "Second unseen."},

            CreateContainer {9u, 0u, ""_us, ContainerControlType::COMBO_PANEL, ContainerControlLayout::VERTICAL, 0u, 0u,
                             false, "Selectable one", "--- none selected ---"},
            CreateLabel {10u, 9u, ""_us, "First unseen."},
            CreateLabel {11u, 9u, ""_us, "Second unseen."},

            CreateCheckbox {12u, 0u, ""_us, "Checked combo", true, {}, InputActionDispatchType::NORMAL},
            CreateCheckbox {13u, 0u, ""_us, "Unchecked combo", false, {}, InputActionDispatchType::NORMAL},

            CreateWindow {
                14u,          ""_us,        "UI"_us, "Right window!",
                true,         true,         true,    true,
                true,         false,        true,    ContainerControlLayout::VERTICAL,
                {1.0f, 1.0f}, {1.0f, 1.0f}, 0,       0u,
                WIDTH / 2,    HEIGHT,       {},      {},
            },

            CreateIntInput (15u, 14u),
            CreateFloatInput (16u, 14u),
            CreateTextInput (17u, 14u),
            CreateImage {18u, 14u, ""_us, 150u, 150u, "Earth.png"_us, {{0.0f, 0.0f}, {2.0f, 2.0f}}},
        }});
}

END_SUITE
