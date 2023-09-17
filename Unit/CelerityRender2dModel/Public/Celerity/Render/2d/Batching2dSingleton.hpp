#pragma once

#include <CelerityRender2dModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Transform2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes 2d batch instance.
/// \details Intended for use only inside CelerityRender2dLogic, therefore undocumented.
struct CelerityRender2dModelApi Batch2d final
{
    std::uint16_t layer = 0u;
    Memory::UniqueString materialInstanceId;
    Container::Vector<UniqueId> sprites;
    Container::Vector<UniqueId> debugShapes;
};

/// \brief Contains full information about viewport with 2d rendering output.
/// \details Intended for use only inside CelerityRender2dLogic, therefore undocumented.
struct CelerityRender2dModelApi ViewportInfoContainer final
{
    Memory::UniqueString name;
    Math::Transform2d cameraTransform;
    Math::Vector2f cameraHalfOrthographicSize {Math::Vector2f::ZERO};

    /// \details By default we take top allocator, but internally we always initialize it to other.
    Container::Vector<Batch2d> batches {Memory::Profiler::AllocationGroup::Top ()};
};

/// \brief Stores internal data 2d batching.
/// \details Intended for use only inside CelerityRender2dLogic, therefore undocumented.
struct CelerityRender2dModelApi Batching2dSingleton final
{
    Batch2d &GetBatch (std::size_t _viewportIndex,
                       std::uint16_t _layer,
                       Memory::UniqueString _materialInstanceId) noexcept;

    void Reset () noexcept;

    Container::Vector<ViewportInfoContainer> viewports {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Batch2d> freeBatches {Memory::Profiler::AllocationGroup::Top ()};

    struct CelerityRender2dModelApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
