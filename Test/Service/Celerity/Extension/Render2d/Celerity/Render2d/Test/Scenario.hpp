#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Math/AxisAlignedBox2d.hpp>
#include <Math/Transform2d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity::Test
{
namespace Tasks
{
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

struct SetCurrentCamera final
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
    uint16_t layer = 0u;
    std::uint64_t visibilityMask = ~0u;
};

struct UpdateSprite final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialInstanceId;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
    uint16_t layer = 0u;
    std::uint64_t visibilityMask = ~0u;
};

struct DeleteSprite final
{
    UniqueId spriteId = INVALID_UNIQUE_ID;
};
}; // namespace Tasks

using Task = Container::Variant<Tasks::CreateCamera,
                                Tasks::UpdateCamera,
                                Tasks::DeleteCamera,
                                Tasks::SetCurrentCamera,
                                Tasks::CreateTransform,
                                Tasks::UpdateTransform,
                                Tasks::DeleteTransform,
                                Tasks::CreateSprite,
                                Tasks::UpdateSprite,
                                Tasks::DeleteSprite>;

using TaskPoint = Container::Vector<Task>;

struct AssetWaitPoint final
{
};

struct ScreenShotPoint final
{
    Memory::UniqueString screenShotId;
};

using Scenario = Container::Vector<Container::Variant<TaskPoint, AssetWaitPoint, ScreenShotPoint>>;

void ExecuteScenario (Scenario _scenario) noexcept;
} // namespace Emergence::Celerity::Test
