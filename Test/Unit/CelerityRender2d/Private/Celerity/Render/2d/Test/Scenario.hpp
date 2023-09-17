#pragma once

#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Math/AxisAlignedBox2d.hpp>
#include <Math/Transform2d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity::Test
{
constexpr std::uint32_t WIDTH = 400u;
constexpr std::uint32_t HEIGHT = 300u;

namespace Tasks
{
struct CreateScreenLikeFrameBuffer final
{
    Memory::UniqueString name;
};

struct CreateViewport final
{
    Memory::UniqueString name;
    Memory::UniqueString targetFrameBuffer;
    std::uint32_t x = 0u;
    std::uint32_t y = 0u;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;
    std::uint32_t clearColor = 0u;
    std::uint16_t sortIndex = 0u;
};

struct UpdateViewport final
{
    Memory::UniqueString name;
    Memory::UniqueString targetFrameBuffer;
    std::uint32_t x = 0u;
    std::uint32_t y = 0u;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;
    std::uint32_t clearColor = 0u;
    std::uint16_t sortIndex = 0u;
};

struct DeleteViewport final
{
    Memory::UniqueString name;
};

struct CreatePostProcessRenderPass final
{
    Memory::UniqueString name;
    Memory::UniqueString materialInstanceId;
};

struct CreateWorldRenderPass final
{
    Memory::UniqueString name;
    UniqueId cameraObjectId = INVALID_UNIQUE_ID;
};

struct UpdateWorldRenderPass final
{
    Memory::UniqueString name;
    UniqueId cameraObjectId = INVALID_UNIQUE_ID;
};

struct CreateCamera
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    float halfOrthographicSize = 0.0f;
    std::uint64_t visibilityMask = ~0u;
};

struct UpdateCamera final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    float halfOrthographicSize = 0.0f;
    std::uint64_t visibilityMask = ~0u;
};

struct DeleteCamera final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
};

struct CreateTransform final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Math::Transform2d localTransform;
};

struct UpdateTransform final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Math::Transform2d localTransform;
};

struct DeleteTransform final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Math::Transform2d localTransform;
};

struct CreateSprite final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId spriteId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialInstanceId;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
    std::uint16_t layer = 0u;
    std::uint64_t visibilityMask = ~0u;
};

struct UpdateSprite final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialInstanceId;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
    std::uint16_t layer = 0u;
    std::uint64_t visibilityMask = ~0u;
};

struct DeleteSprite final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
};

struct CreateDebugShape final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId debugShapeId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialInstanceId;
    Math::Vector2f translation = Math::Vector2f::ZERO;
    float rotation = 0.0f;
    DebugShape2d shape;
};

struct UpdateDebugShape final
{
    UniqueId debugShapeId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialInstanceId;
    Math::Vector2f translation = Math::Vector2f::ZERO;
    float rotation = 0.0f;
    DebugShape2d shape;
};

struct DeleteDebugShape final
{
    UniqueId debugShapeId = INVALID_UNIQUE_ID;
};

struct CreateSpriteAnimation final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId spriteId = INVALID_UNIQUE_ID;
    Memory::UniqueString animationId;
    std::uint64_t currentTimeNs = 0u;
    bool tickTime = true;
    bool loop = false;
    bool flipU = false;
    bool flipV = false;
};

struct UpdateSpriteAnimation final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
    Memory::UniqueString animationId;
    std::uint64_t currentTimeNs = 0u;
    bool tickTime = true;
    bool loop = false;
    bool flipU = false;
    bool flipV = false;
};

struct DeleteSpriteAnimation final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
};
}; // namespace Tasks

using Task = Container::Variant<Tasks::CreateScreenLikeFrameBuffer,
                                Tasks::CreateViewport,
                                Tasks::UpdateViewport,
                                Tasks::DeleteViewport,
                                Tasks::CreatePostProcessRenderPass,
                                Tasks::CreateWorldRenderPass,
                                Tasks::UpdateWorldRenderPass,
                                Tasks::CreateCamera,
                                Tasks::UpdateCamera,
                                Tasks::DeleteCamera,
                                Tasks::CreateTransform,
                                Tasks::UpdateTransform,
                                Tasks::DeleteTransform,
                                Tasks::CreateSprite,
                                Tasks::UpdateSprite,
                                Tasks::DeleteSprite,
                                Tasks::CreateDebugShape,
                                Tasks::UpdateDebugShape,
                                Tasks::DeleteDebugShape,
                                Tasks::CreateSpriteAnimation,
                                Tasks::UpdateSpriteAnimation,
                                Tasks::DeleteSpriteAnimation>;

using TaskPoint = Container::Vector<Task>;

struct AssetWaitPoint final
{
};

struct ScreenShotPoint final
{
    Memory::UniqueString screenShotId;
};

struct FrameSkipPoint final
{
    std::uint64_t frameCount = 0u;
};

using Scenario = Container::Vector<Container::Variant<TaskPoint, AssetWaitPoint, ScreenShotPoint, FrameSkipPoint>>;

void ExecuteScenario (Scenario _scenario) noexcept;
} // namespace Emergence::Celerity::Test
