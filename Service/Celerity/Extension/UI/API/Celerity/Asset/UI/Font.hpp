#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
constexpr char FONT_SIZE_SEPARATOR = '#';

struct Font final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Font);

    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    Render::Backend::Texture atlasTexture;

    void *nativeHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId size;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
