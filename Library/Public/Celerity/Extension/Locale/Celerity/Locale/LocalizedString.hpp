#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores localized utf8 string with its localization key.
struct LocalizedString final
{
    /// \brief Unique key that identifies this localized value.
    Memory::UniqueString key;

    /// \brief Utf8 string value in LocaleSingleton::loadedLocale.
    Container::Utf8String value;

    struct Reflection final
    {
        StandardLayout::FieldId key;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
