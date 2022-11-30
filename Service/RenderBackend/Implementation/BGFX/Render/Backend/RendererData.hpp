#pragma once

#include <cstdint>

#include <Container/Vector.hpp>

#include <Render/Backend/Allocator.hpp>

namespace Emergence::Render::Backend
{
struct RendererData final
{
    std::atomic_unsigned_lock_free viewportIndexCounter = 0u;
    Container::Vector<uint16_t> viewNativeIds {GetAllocationGroup ()};
};
} // namespace Emergence::Render::Backend
