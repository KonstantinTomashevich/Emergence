#pragma once

#include <CelerityLocaleApi.hpp>

#include <Celerity/Locale/LocalizedString.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents structure of file that stores everything about specific locale.
struct CelerityLocaleApi LocaleConfiguration final
{
    /// \brief Array of strings, localized for this locale.
    Container::Vector<LocalizedString> strings {Memory::Profiler::AllocationGroup::Top ()};

    struct CelerityLocaleApi Reflection final
    {
        StandardLayout::FieldId strings;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
