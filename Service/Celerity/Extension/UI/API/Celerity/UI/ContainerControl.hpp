#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class ContainerControlType
{
    PANEL,
    COLLAPSING_PANEL,
    COMBO_PANEL,
};

enum class ContainerControlLayout
{
    VERTICAL,
    HORIZONTAL,
};

struct ContainerControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    ContainerControlType type = ContainerControlType::PANEL;
    ContainerControlLayout layout = ContainerControlLayout::VERTICAL;

    /// \invariant Only applicable for ContainerType::PANEL.
    uint32_t width = 0u;

    /// \invariant Only applicable for ContainerType::PANEL.
    uint32_t height = 0u;

    /// \invariant Only applicable for ContainerType::COLLAPSING_PANEL and ContainerType::COMBO_PANEL.
    Container::Utf8String label;

    /// \invariant Only applicable for ContainerType::COMBO_PANEL.
    Container::Utf8String preview;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId layout;

        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
