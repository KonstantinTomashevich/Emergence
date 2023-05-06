#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Transform2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Batch2d final
{
    uint16_t layer = 0u;
    Memory::UniqueString materialInstanceId;
    Container::Vector<UniqueId> sprites;
    Container::Vector<UniqueId> debugShapes;
};

struct ViewportInfoContainer final
{
    Memory::UniqueString name;
    Math::Transform2d cameraTransform;
    Math::Vector2f cameraHalfOrthographicSize {Math::Vector2f::ZERO};
    Container::Vector<Batch2d> batches;
};

struct Batching2dSingleton final
{
    Batch2d &GetBatch (size_t _viewportIndex, uint16_t _layer, Memory::UniqueString _materialInstanceId) noexcept;

    void Reset () noexcept;

    Container::Vector<ViewportInfoContainer> viewports {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Batch2d> freeBatches {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
