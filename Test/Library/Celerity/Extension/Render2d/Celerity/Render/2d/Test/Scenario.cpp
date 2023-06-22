#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>
#include <thread>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimationManagement.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
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
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
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

#include <Resource/Provider/ResourceProvider.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Testing/FileSystemTestUtility.hpp>
#include <Testing/ResourceContextHolder.hpp>
#include <Testing/SDLContextHolder.hpp>
#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Reader.hpp>

namespace Emergence
{
namespace Testing
{
Container::MappingRegistry GetSupportedResourceTypes () noexcept
{
    Container::MappingRegistry registry;
    registry.Register (Celerity::MaterialAsset::Reflect ().mapping);
    registry.Register (Celerity::MaterialInstanceAsset::Reflect ().mapping);
    registry.Register (Celerity::Sprite2dUvAnimationAsset::Reflect ().mapping);
    registry.Register (Celerity::TextureAsset::Reflect ().mapping);
    return registry;
}
} // namespace Testing

namespace Celerity::Test
{
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

    InsertLongTermQuery insertDebugShape;
    ModifyValueQuery modifyDebugShape;

    InsertLongTermQuery insertSpriteAnimation;
    ModifyValueQuery modifySpriteAnimation;

    std::size_t currentPointIndex = 0u;
    Scenario scenario;
    std::uint32_t framesWaiting = 0u;
    bool *finishedOutput = nullptr;
    Memory::UniqueString pendingScreenShot;
    std::uint64_t framesLeftToSkip = 0u;

    // We need to wait one frame after checking if all assets were loaded, because
    // new asset usages might be created as a result of current frame asset loading.
    bool allAssetsWereLoadedDuringPreviousFrame = false;
};

ScenarioExecutor::ScenarioExecutor (TaskConstructor &_constructor, Scenario _scenario, bool *_finishedOutput) noexcept
    : TaskExecutorBase (_constructor),

      fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),
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

      insertDebugShape (INSERT_LONG_TERM (DebugShape2dComponent)),
      modifyDebugShape (MODIFY_VALUE_1F (DebugShape2dComponent, debugShapeId)),

      insertSpriteAnimation (INSERT_LONG_TERM (Sprite2dUvAnimationComponent)),
      modifySpriteAnimation (MODIFY_VALUE_1F (Sprite2dUvAnimationComponent, spriteId)),

      scenario (std::move (_scenario)),
      finishedOutput (_finishedOutput)
{
    _constructor.DependOn (AssetManagement::Checkpoint::FINISHED);
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
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
            if (allAssetsWereLoadedDuringPreviousFrame)
            {
                LOG ("Finished loading all assets!");
                ++currentPointIndex;
                framesWaiting = 0u;
            }

            allAssetsWereLoadedDuringPreviousFrame = true;
        }
        else
        {
            allAssetsWereLoadedDuringPreviousFrame = false;
            ++framesWaiting;
            // ~5 seconds target FPS loading time cap.
            REQUIRE ((framesWaiting < 300u));
        }
    }
    else if (auto *screenShotPoint = std::get_if<ScreenShotPoint> (&scenario[currentPointIndex]))
    {
        LOG ("Taking screen shot \"", screenShotPoint->screenShotId, "\"...");
        Emergence::Render::Backend::TakePngScreenshot (EMERGENCE_BUILD_STRING (screenShotPoint->screenShotId, ".png"));
        pendingScreenShot = screenShotPoint->screenShotId;
        ++currentPointIndex;
    }
    else if (auto *frameSkipPoint = std::get_if<FrameSkipPoint> (&scenario[currentPointIndex]))
    {
        LOG ("Skipping frames...");
        if (framesLeftToSkip == 0u)
        {
            framesLeftToSkip = frameSkipPoint->frameCount;
        }
        else if (0u == --framesLeftToSkip)
        {
            ++currentPointIndex;
        }
    }
}

void ScenarioExecutor::ExecuteTasks (TaskPoint *_point) noexcept
{
    for (const Task &task : *_point)
    {
        std::visit (
            [this] (const auto &_task)
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
                else if constexpr (std::is_same_v<Type, Tasks::CreateDebugShape>)
                {
                    LOG ("Creating debug shape on object ", _task.objectId, " with id ", _task.debugShapeId, ".");
                    auto cursor = insertDebugShape.Execute ();
                    auto *debugShape = static_cast<DebugShape2dComponent *> (++cursor);
                    debugShape->objectId = _task.objectId;
                    debugShape->debugShapeId = _task.debugShapeId;
                    debugShape->materialInstanceId = _task.materialInstanceId;
                    debugShape->translation = _task.translation;
                    debugShape->rotation = _task.rotation;
                    debugShape->shape = _task.shape;
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateDebugShape>)
                {
                    LOG ("Updating debug shape with id ", _task.debugShapeId, ".");
                    auto cursor = modifyDebugShape.Execute (&_task.debugShapeId);
                    auto *debugShape = static_cast<DebugShape2dComponent *> (*cursor);
                    REQUIRE (debugShape);
                    debugShape->debugShapeId = _task.debugShapeId;
                    debugShape->materialInstanceId = _task.materialInstanceId;
                    debugShape->translation = _task.translation;
                    debugShape->rotation = _task.rotation;
                    debugShape->shape = _task.shape;
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteDebugShape>)
                {
                    LOG ("Deleting debug shape with id ", _task.debugShapeId, ".");
                    auto cursor = modifyDebugShape.Execute (&_task.debugShapeId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateSpriteAnimation>)
                {
                    LOG ("Creating sprite animation on object ", _task.objectId, " and sprite ", _task.spriteId, ".");
                    auto cursor = insertSpriteAnimation.Execute ();
                    auto *animation = static_cast<Sprite2dUvAnimationComponent *> (++cursor);
                    animation->objectId = _task.objectId;
                    animation->spriteId = _task.spriteId;
                    animation->animationId = _task.animationId;
                    animation->currentTimeNs = _task.currentTimeNs;
                    animation->tickTime = _task.tickTime;
                    animation->loop = _task.loop;
                    animation->flipU = _task.flipU;
                    animation->flipV = _task.flipV;
                }
                else if constexpr (std::is_same_v<Type, Tasks::UpdateSpriteAnimation>)
                {
                    LOG ("Updating  sprite animation on sprite with id ", _task.spriteId, ".");
                    auto cursor = modifySpriteAnimation.Execute (&_task.spriteId);
                    auto *animation = static_cast<Sprite2dUvAnimationComponent *> (*cursor);
                    REQUIRE (animation);
                    animation->animationId = _task.animationId;
                    animation->currentTimeNs = _task.currentTimeNs;
                    animation->tickTime = _task.tickTime;
                    animation->loop = _task.loop;
                    animation->flipU = _task.flipU;
                    animation->flipV = _task.flipV;
                }
                else if constexpr (std::is_same_v<Type, Tasks::DeleteSpriteAnimation>)
                {
                    LOG ("Deleting sprite animation from sprite with id ", _task.spriteId, ".");
                    auto cursor = modifySpriteAnimation.Execute (&_task.spriteId);
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
    std::this_thread::sleep_for (std::chrono::milliseconds {300u});
    LOG ("Starting image check.");

    VirtualFileSystem::Reader expectedReader {
        VirtualFileSystem::Entry {
            Testing::ResourceContextHolder::Get ().virtualFileSystem,
            EMERGENCE_BUILD_STRING (Testing::ResourceContextHolder::TEST_DIRECTORY, VirtualFileSystem::PATH_SEPARATOR,
                                    "Render2dTest", VirtualFileSystem::PATH_SEPARATOR, "Expectation",
                                    VirtualFileSystem::PATH_SEPARATOR, _id, ".png")},
        VirtualFileSystem::OpenMode::BINARY};
    REQUIRE (expectedReader);

    VirtualFileSystem::Reader resultReader {
        VirtualFileSystem::Entry {Testing::ResourceContextHolder::Get ().virtualFileSystem,
                                  EMERGENCE_BUILD_STRING (Testing::ResourceContextHolder::TEST_OUTPUT_DIRECTORY,
                                                          VirtualFileSystem::PATH_SEPARATOR, _id, ".png")},
        VirtualFileSystem::OpenMode::BINARY};
    REQUIRE (resultReader);

    Testing::CheckStreamEquality (expectedReader.InputStream (), resultReader.InputStream (), 0.02f);
}

void ExecuteScenario (Scenario _scenario) noexcept
{
    using namespace Memory::Literals;

    World world {Emergence::Memory::UniqueString {"TestWorld"}, {{1.0f / 60.0f}}};
    AssetReferenceBindingList binding {GetAssetBindingAllocationGroup ()};
    GetRender2dAssetUsage (binding);
    GetRenderFoundationAssetUsage (binding);
    AssetReferenceBindingEventMap assetReferenceBindingEventMap;

    {
        EventRegistrar registrar {&world};
        assetReferenceBindingEventMap = RegisterAssetEvents (registrar, binding);
        RegisterTransform2dEvents (registrar);
        RegisterTransformCommonEvents (registrar);
        RegisterRender2dEvents (registrar);
        RegisterRenderFoundationEvents (registrar);
    }

    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};
    PipelineBuilder pipelineBuilder {world.GetRootView ()};
    bool scenarioFinished = false;

    pipelineBuilder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    AssetManagement::AddToNormalUpdate (pipelineBuilder, binding, assetReferenceBindingEventMap);
    TransformHierarchyCleanup::Add2dToNormalUpdate (pipelineBuilder);
    MaterialInstanceManagement::AddToNormalUpdate (
        pipelineBuilder, &Testing::ResourceContextHolder::Get ().resourceProvider, assetReferenceBindingEventMap);
    MaterialManagement::AddToNormalUpdate (pipelineBuilder, &Testing::ResourceContextHolder::Get ().resourceProvider,
                                           assetReferenceBindingEventMap);
    RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    Rendering2d::AddToNormalUpdate (pipelineBuilder, worldBox);
    Sprite2dUvAnimationManagement::AddToNormalUpdate (
        pipelineBuilder, &Testing::ResourceContextHolder::Get ().resourceProvider, assetReferenceBindingEventMap);
    TextureManagement::AddToNormalUpdate (pipelineBuilder, &Testing::ResourceContextHolder::Get ().resourceProvider,
                                          assetReferenceBindingEventMap);
    TransformVisualSync::Add2dToNormalUpdate (pipelineBuilder);
    pipelineBuilder.AddTask ("ScenarioExecutor"_us)
        .SetExecutor<ScenarioExecutor> (std::move (_scenario), &scenarioFinished);
    REQUIRE (pipelineBuilder.End ());

    constexpr std::uint64_t SIMULATED_TIME_STEP_NS = 10000000u;
    std::uint64_t timeOverride = 0u;

    while (!scenarioFinished)
    {
        Testing::SDLContextHolder::Get ().Frame ();
        Time::Override (timeOverride);
        world.Update ();
        timeOverride += SIMULATED_TIME_STEP_NS;
    }
}
} // namespace Celerity::Test
} // namespace Emergence
