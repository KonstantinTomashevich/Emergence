#pragma once

#include <CelerityLocaleModelApi.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores localized utf8 string with its localization key.
struct CelerityLocaleModelApi LocalizedString final
{
    /// \brief Unique key that identifies this localized value.
    Memory::UniqueString key;

    /// \brief Utf8 string value in LocaleSingleton::loadedLocale.
    Container::Utf8String value;

    struct CelerityLocaleModelApi Reflection final
    {
        StandardLayout::FieldId key;
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
