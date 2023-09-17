#pragma once

#include <CelerityLocaleModelApi.hpp>

#include <Celerity/Locale/LocalizedString.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents structure of file that stores everything about specific locale.
struct CelerityLocaleModelApi LocaleConfiguration final
{
    /// \brief Array of strings, localized for this locale.
    Container::Vector<LocalizedString> strings {Memory::Profiler::AllocationGroup::Top ()};

    struct CelerityLocaleModelApi Reflection final
    {
        StandardLayout::FieldId strings;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
