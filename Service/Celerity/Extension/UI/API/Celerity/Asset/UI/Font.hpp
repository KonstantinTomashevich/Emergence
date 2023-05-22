#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Content of font file that stores all the information about this font.
struct FontAsset final
{
    /// \brief Id of file with third party format that contains font data, like "MyFont.ttf".
    Memory::UniqueString fontId;

    struct Reflection final
    {
        StandardLayout::FieldId fontId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
