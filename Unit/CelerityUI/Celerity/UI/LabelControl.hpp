#pragma once

#include <CelerityUIApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents control that displays simple text.
struct CelerityUIApi LabelControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Key to LocalizedString that is displayed.
    Memory::UniqueString labelKey;

    /// \brief Text that is displayed. Used instead of ::labelKey if present.
    Container::Utf8String label;

    struct CelerityUIApi Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId labelKey;
        StandardLayout::FieldId label;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
