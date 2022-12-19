#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Font final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    float size = 12.0f;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId size;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
