#pragma once

#include <Memory/UniqueString.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents successfully loaded 2d texture asset.
struct Texture final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Underlying implementation object.
    /// \details Field is mutable, because render backend objects are technically handles and by modifying them
    ///          we work with underlying implementation that operates under different read-write ruleset.
    mutable Render::Backend::Texture texture;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
