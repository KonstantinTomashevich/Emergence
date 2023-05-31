#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes how ContainerControl is visually and logically represented.
enum class ContainerControlType
{
    /// \brief All elements are visible and control works as unmovable subwindow with optional border.
    PANEL,

    /// \brief A ::PANEL that has title and can be collapsed like window, but its size is automatically packed.
    COLLAPSING_PANEL,

    /// \brief A ::PANEL that operates like combo box with label and preview value.
    COMBO_PANEL,
};

/// \brief Describes how child nodes are positioned in ContainerControl.
enum class ContainerControlLayout
{
    /// \brief Child nodes are placed one after another in vertical line.
    VERTICAL,

    /// \brief Child nodes are placed one after another in horizontal line.
    HORIZONTAL,
};

/// \brief Control that groups child UINode's and handles their placement and visibility.
struct ContainerControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Describes type of container.
    ContainerControlType type = ContainerControlType::PANEL;

    /// \brief Describes how container calculates element positions.
    ContainerControlLayout layout = ContainerControlLayout::VERTICAL;

    /// \brief Container control fixed width.
    /// \invariant Only applicable for ContainerType::PANEL.
    std::uint32_t width = 0u;

    /// \brief Container control fixed height.
    /// \invariant Only applicable for ContainerType::PANEL.
    std::uint32_t height = 0u;

    /// \brief Whether container control has visible border.
    /// \invariant Only applicable for ContainerType::PANEL.
    bool border = false;

    /// \brief Key to LocalizedString that describes this container object.
    /// \invariant Only applicable for ContainerType::COLLAPSING_PANEL and ContainerType::COMBO_PANEL.
    Memory::UniqueString labelKey;

    /// \brief Title that describes this container content. Used instead of ::labelKey if present.
    /// \invariant Only applicable for ContainerType::COLLAPSING_PANEL and ContainerType::COMBO_PANEL.
    Container::Utf8String label;

    /// \brief Key to LocalizedString that serves as preview value that describes current state of this container.
    /// \invariant Only applicable for ContainerType::COMBO_PANEL.
    Memory::UniqueString previewKey;

    /// \brief Preview value that describes current state of this container. Used instead of ::previewKey if present.
    /// \invariant Only applicable for ContainerType::COMBO_PANEL.
    Container::Utf8String preview;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId layout;

        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::FieldId border;

        StandardLayout::FieldId labelKey;
        StandardLayout::FieldId label;
        StandardLayout::FieldId previewKey;
        StandardLayout::FieldId preview;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
