#pragma once

#include <Memory/UniqueString.hpp>

#include <Render/Backend/FrameBuffer.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Holds constructed frame buffer instance. Uses Asset framework for reference counting.
struct FrameBuffer final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Underlying implementation object.
    Render::Backend::FrameBuffer frameBuffer;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
