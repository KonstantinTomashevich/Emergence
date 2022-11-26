#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents successfully loaded 2d texture asset.
struct Texture2d final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Texture2d);

    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Implementation-specific native handle.
    uint64_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
