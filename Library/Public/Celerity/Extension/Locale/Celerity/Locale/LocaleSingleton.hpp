#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores global configuration and state of localization routine.
struct LocaleSingleton final
{
    /// \brief Target locale to be loaded and applied. Edit this field if you need to change locale.
    Memory::UniqueString targetLocale;

    /// \brief Locale that is currently loaded. Designed to be readonly for users.
    Memory::UniqueString loadedLocale;

    /// \brief Locale that is currently being loaded if any. Designed to be readonly for users.
    Memory::UniqueString loadingLocale;

    struct Reflection final
    {
        StandardLayout::FieldId targetLocale;
        StandardLayout::FieldId loadedLocale;
        StandardLayout::FieldId loadingLocale;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
