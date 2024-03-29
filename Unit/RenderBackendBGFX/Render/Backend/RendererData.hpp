#pragma once

#include <atomic>
#include <cstdint>

#include <Container/Vector.hpp>

#include <Render/Backend/Allocator.hpp>

namespace Emergence::Render::Backend
{
struct RendererData final
{
    std::atomic_uintptr_t viewportIndexCounter = 0u;
    Container::Vector<std::uint16_t> viewNativeIds {GetAllocationGroup ()};
};
} // namespace Emergence::Render::Backend
