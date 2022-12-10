#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/Test/Scenario.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/World.hpp>

#include <Render/Backend/Configuration.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
class ContextHolder final
{
public:
    static void Frame () noexcept;

    ContextHolder (const ContextHolder &_other) = delete;

    ContextHolder (ContextHolder &&_other) = delete;

    ContextHolder &operator= (const ContextHolder &_other) = delete;

    ContextHolder &operator= (ContextHolder &&_other) = delete;

private:
    ContextHolder () noexcept;

    ~ContextHolder () noexcept;

    SDL_Window *window = nullptr;
};

void ContextHolder::Frame () noexcept
{
    static ContextHolder contextHolder;
    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
        // Just poll all events...
    }
}

ContextHolder::ContextHolder () noexcept
{
    uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;
    window =
        SDL_CreateWindow ("Platformed2dDemo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int> (WIDTH),
                          static_cast<int> (HEIGHT), static_cast<SDL_WindowFlags> (windowFlags));

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
    config.width = WIDTH;
    config.height = HEIGHT;
    config.vsync = true;
    Emergence::Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

ContextHolder::~ContextHolder () noexcept
{
    if (window)
    {
        Emergence::Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}

class ScenarioExecutor final : public TaskExecutorBase<ScenarioExecutor>
{
public:
    ScenarioExecutor (TaskConstructor &_constructor, Scenario _scenario, bool *_finishedOutput) noexcept;

    void Execute () noexcept;

private:
    void ExecuteTasks (TaskPoint *_point) noexcept;

    static void CompareScreenShots (Memory::UniqueString &_id) noexcept;

    FetchSingletonQuery fetchAssetManager;
    ModifySingletonQuery modifyRender;

    InsertLongTermQuery insertViewport;
    InsertLongTermQuery insertWorldPass;
    ModifyValueQuery modifyViewport;
    ModifyValueQuery modifyWorldPass;

    InsertLongTermQuery insertCamera;
    ModifyValueQuery modifyCamera;

    InsertLongTermQuery insertTransform;
    ModifyValueQuery modifyTransform;

    InsertLongTermQuery insertSprite;
    ModifyValueQuery modifySprite;

    std::size_t currentPointIndex = 0u;
    Scenario scenario;
    uint32_t framesWaiting = 0u;
    bool *finishedOutput = nullptr;
    Memory::UniqueString pendingScreenShot;
};

ScenarioExecutor::ScenarioExecutor (TaskConstructor &_constructor, Scenario _scenario, bool *_finishedOutput) noexcept
    : fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),
      modifyRender (MODIFY_SINGLETON (Render2dSingleton)),

      insertViewport (INSERT_LONG_TERM (Viewport)),
      insertWorldPass (INSERT_LONG_TERM (World2dRenderPass)),
      modifyViewport (MODIFY_VALUE_1F (Viewport, name)),
      modifyWorldPass (MODIFY_VALUE_1F (World2dRenderPass, name)),

      insertCamera (INSERT_LONG_TERM (Camera2dComponent)),
      modifyCamera (MODIFY_VALUE_1F (Camera2dComponent, objectId)),

      insertTransform (INSERT_LONG_TERM (Transform2dComponent)),
      modifyTransform (MODIFY_VALUE_1F (Transform2dComponent, objectId)),

      insertSprite (INSERT_LONG_TERM (Sprite2dComponent)),
      modifySprite (MODIFY_VALUE_1F (Sprite2dComponent, spriteId)),

      scenario (std::move (_scenario)),
      finishedOutput (_finishedOutput)
{
    _constructor.DependOn (AssetManagement::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (
        Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_STARTED);
    _constructor.MakeDependencyOf (RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (TransformVisualSync::Checkpoint::STARTED);
}

void ScenarioExecutor::Execute () noexcept
{
    if (*pendingScreenShot)
    {
        CompareScreenShots (pendingScreenShot);
        pendingScreenShot = Memory::UniqueString {};
    }

    if (currentPointIndex >= scenario.size ())
    {
        *finishedOutput = true;
        return;
    }

    if (auto *taskPoint = std::get_if<TaskPoint> (&scenario[currentPointIndex]))
    {
        ExecuteTasks (taskPoint);
        ++currentPointIndex;
    }
    else if (std::get_if<AssetWaitPoint> (&scenario[currentPointIndex]))
    {
        LOG ("Waiting for all assets to finish loading...");
        auto assetManagerCursor = fetchAssetManager.Execute ();
        const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);

        if (assetManager->assetsLeftToLoad == 0u)
        {
            LOG ("Finished loading all assets!");
            ++currentPointIndex;
            framesWaiting = 0u;
        }
        else
        {
            ++framesWaiting;
            // ~5 seconds target FPS loading time cap.
            REQUIRE (framesWaiting < 300u);
        }
    }
    else if (auto *screenShotPoint = std::get_if<ScreenShotPoint> (&scenario[currentPointIndex]))
    {
        LOG ("Taking screen shot \"", screenShotPoint->screenShotId, "\"...");
        Emergence::Render::Backend::TakePngScreenshot (EMERGENCE_BUILD_STRING (screenShotPoint->screenShotId, ".png"));
        pendingScreenShot = screenShotPoint->screenShotId;
        ++currentPointIndex;
    }
}

void ScenarioExecutor::ExecuteTasks (TaskPoint *_point) noexcept
{
    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);

    for (const Task &task : *_point)
    {
        std::visit (
            [this, assetManager] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, Tasks::CreateViewport>)
                {
                    LOG ("Creating viewport \"", _task.name, "\".");
                    auto cursor = insertViewport.Execute ();
                    auto *viewport = static_cast<Viewport *> (++cursor);

                    viewport->name = _task.name;
                    viewport->x = _task.x;
                    viewport->y = _task.y;
                    viewport->width = _task.width;
                    viewport->height = _task.height;
                    viewport->clearColor = _task.clearColor;
                    viewport->sortIndex = _task.sortIndex;

                    auto passCursor = insertWorldPass.Execute ();
                    auto *pass = static_cast<World2dRenderPass *> (++passCursor);

                    pass->name = viewport->name;
                    pass->cameraObjectId = _task.cameraObjectId;
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateViewport>)
                {
                    LOG ("Updating viewport \"", _task.name, "\".");
                    auto cursor = modifyViewport.Execute (&_task.name);
                    auto *viewport = static_cast<Viewport *> (*cursor);

                    viewport->x = _task.x;
                    viewport->y = _task.y;
                    viewport->width = _task.width;
                    viewport->height = _task.height;
                    viewport->clearColor = _task.clearColor;
                    viewport->sortIndex = _task.sortIndex;

                    auto passCursor = modifyWorldPass.Execute (&_task.name);
                    auto *pass = static_cast<World2dRenderPass *> (*passCursor);
                    pass->cameraObjectId = _task.cameraObjectId;
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteViewport>)
                {
                    LOG ("Deleting viewport \"", _task.name, "\".");
                    auto cursor = modifyViewport.Execute (&_task.name);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateCamera>)
                {
                    LOG ("Creating camera on object ", _task.objectId, ".");
                    auto cursor = insertCamera.Execute ();
                    auto *camera = static_cast<Camera2dComponent *> (++cursor);
                    camera->objectId = _task.objectId;
                    camera->halfOrthographicSize = _task.halfOrthographicSize;
                    camera->visibilityMask = _task.visibilityMask;
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateCamera>)
                {
                    LOG ("Updating camera on object ", _task.objectId, ".");
                    auto cursor = modifyCamera.Execute (&_task.objectId);
                    auto *camera = static_cast<Camera2dComponent *> (*cursor);
                    camera->halfOrthographicSize = _task.halfOrthographicSize;
                    camera->visibilityMask = _task.visibilityMask;
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteCamera>)
                {
                    LOG ("Deleting camera from object ", _task.objectId, ".");
                    auto cursor = modifyCamera.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateTransform>)
                {
                    LOG ("Creating transform on object ", _task.objectId, ".");
                    auto cursor = insertTransform.Execute ();
                    auto *transform = static_cast<Transform2dComponent *> (++cursor);
                    transform->SetObjectId (_task.objectId);
                    transform->SetParentObjectId (_task.parentId);
                    transform->SetVisualLocalTransform (_task.localTransform);
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateTransform>)
                {
                    LOG ("Updating transform on object ", _task.objectId, ".");
                    auto cursor = modifyTransform.Execute (&_task.objectId);
                    auto *transform = static_cast<Transform2dComponent *> (*cursor);
                    transform->SetParentObjectId (_task.parentId);
                    transform->SetVisualLocalTransform (_task.localTransform);
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteTransform>)
                {
                    LOG ("Deleting transform from object ", _task.objectId, ".");
                    auto cursor = modifyTransform.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateSprite>)
                {
                    LOG ("Creating sprite on object ", _task.objectId, " with id ", _task.spriteId, ".");
                    auto cursor = insertSprite.Execute ();
                    auto *sprite = static_cast<Sprite2dComponent *> (++cursor);
                    sprite->objectId = _task.objectId;
                    sprite->spriteId = _task.spriteId;
                    sprite->assetUserId = assetManager->GenerateAssetUserId ();
                    sprite->materialInstanceId = _task.materialInstanceId;
                    sprite->uv = _task.uv;
                    sprite->halfSize = _task.halfSize;
                    sprite->layer = _task.layer;
                    sprite->visibilityMask = _task.visibilityMask;
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateSprite>)
                {
                    LOG ("Updating sprite with id", _task.spriteId, ".");
                    auto cursor = modifySprite.Execute (&_task.spriteId);
                    auto *sprite = static_cast<Sprite2dComponent *> (*cursor);
                    sprite->spriteId = _task.spriteId;
                    sprite->materialInstanceId = _task.materialInstanceId;
                    sprite->uv = _task.uv;
                    sprite->halfSize = _task.halfSize;
                    sprite->layer = _task.layer;
                    sprite->visibilityMask = _task.visibilityMask;
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteSprite>)
                {
                    LOG ("Deleting sprite with id ", _task.spriteId, ".");
                    auto cursor = modifySprite.Execute (&_task.spriteId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
            },
            task);
    }
}

void ScenarioExecutor::CompareScreenShots (Memory::UniqueString &_id) noexcept
{
    LOG ("Waiting for screenshot...");
    while (!std::filesystem::exists (EMERGENCE_BUILD_STRING (_id, ".png")))
    {
        std::this_thread::yield ();
    }

    LOG ("Giving renderer some time to write image...");
    std::this_thread::sleep_for (std::chrono::milliseconds {200u});
    LOG ("Starting image check.");

    std::ifstream exampleInput {EMERGENCE_BUILD_STRING ("Render2dTestResources/Expectation/", _id, ".png"),
                                std::ios::binary};
    REQUIRE (exampleInput);
    std::ifstream testInput {EMERGENCE_BUILD_STRING (_id, ".png"), std::ios::binary};
    REQUIRE (testInput);

    exampleInput.seekg (0u, std::ios::end);
    std::streamsize exampleSize = exampleInput.tellg ();
    exampleInput.seekg (0u, std::ios::beg);

    testInput.seekg (0u, std::ios::end);
    std::streamsize testSize = testInput.tellg ();
    testInput.seekg (0u, std::ios::beg);

    CHECK_EQUAL (exampleSize, testSize);
    if (exampleSize != testSize)
    {
        return;
    }

    constexpr auto BUFFER_SIZE = static_cast<std::streamsize> (16u * 1024u);
    std::streamsize read = 0u;
    std::array<uint8_t, BUFFER_SIZE> exampleBuffer;
    std::array<uint8_t, BUFFER_SIZE> testBuffer;

    while (read < exampleSize)
    {
        const std::streamsize toRead = std::min (BUFFER_SIZE, exampleSize - read);
        read += toRead;

        REQUIRE (exampleInput.read (reinterpret_cast<char *> (exampleBuffer.data ()), toRead));
        REQUIRE (testInput.read (reinterpret_cast<char *> (testBuffer.data ()), toRead));
        CHECK (memcmp (exampleBuffer.data (), testBuffer.data (), toRead) == 0);
    }
}

void ExecuteScenario (Scenario _scenario) noexcept
{
    using namespace Memory::Literals;

    Emergence::Celerity::World world {Emergence::Memory::UniqueString {"TestWorld"}, {{1.0f / 60.0f}}};
    Emergence::Celerity::AssetReferenceBindingList binding {Emergence::Celerity::GetAssetBindingAllocationGroup ()};
    Emergence::Celerity::GetRender2dAssetUsage (binding);
    Emergence::Celerity::GetRenderFoundationAssetUsage (binding);
    Emergence::Celerity::AssetReferenceBindingEventMap assetReferenceBindingEventMap;

    {
        Emergence::Celerity::EventRegistrar registrar {&world};
        assetReferenceBindingEventMap = Emergence::Celerity::RegisterAssetEvents (registrar, binding);
        Emergence::Celerity::RegisterTransform2dEvents (registrar);
        Emergence::Celerity::RegisterTransformCommonEvents (registrar);
        Emergence::Celerity::RegisterRender2dEvents (registrar);
        Emergence::Celerity::RegisterRenderFoundationEvents (registrar);
    }

    constexpr uint64_t MAX_LOADING_TIME_NS = 16000000;
    static const Emergence::Memory::UniqueString testMaterialInstancesPath {"Render2dTestResources/MaterialInstances"};
    static const Emergence::Memory::UniqueString testMaterialsPath {"Render2dTestResources/Materials"};
    static const Emergence::Memory::UniqueString engineMaterialsPath {"Render2dResources/Materials"};
    static const Emergence::Memory::UniqueString testShadersPath {"Render2dTestResources/Shaders"};
    static const Emergence::Memory::UniqueString engineShadersPath {"Render2dResources/Shaders"};
    static const Emergence::Memory::UniqueString testTexturesPath {"Render2dTestResources/Textures"};
    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};
    bool scenarioFinished = false;

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Emergence::Celerity::AssetManagement::AddToNormalUpdate (pipelineBuilder, binding, assetReferenceBindingEventMap);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::MaterialInstanceManagement::AddToNormalUpdate (
        pipelineBuilder, {testMaterialInstancesPath}, MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::MaterialManagement::AddToNormalUpdate (
        pipelineBuilder, {testMaterialsPath, engineMaterialsPath}, {testShadersPath, engineShadersPath},
        MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (pipelineBuilder, worldBox);
    Emergence::Celerity::TextureManagement::AddToNormalUpdate (pipelineBuilder, {testTexturesPath}, MAX_LOADING_TIME_NS,
                                                               assetReferenceBindingEventMap);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (pipelineBuilder);
    pipelineBuilder.AddTask ("ScenarioExecutor"_us)
        .SetExecutor<ScenarioExecutor> (std::move (_scenario), &scenarioFinished);
    REQUIRE (pipelineBuilder.End ());

    while (!scenarioFinished)
    {
        ContextHolder::Frame ();
        world.Update ();
    }
}
} // namespace Emergence::Celerity::Test
