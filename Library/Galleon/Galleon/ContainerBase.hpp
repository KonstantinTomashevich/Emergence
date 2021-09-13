#pragma once

#include <atomic>
#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon
{
class ContainerBase : public Handling::HandleableBase
{
public:
    /// Containers are managed by CargoDeck, therefore they can not be copied or moved.
    ContainerBase (const ContainerBase &_other) = delete;

    ContainerBase (ContainerBase &&_other) = delete;

    const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// Containers are managed by CargoDeck, therefore they can not be assigned.
    EMERGENCE_DELETE_ASSIGNMENT (ContainerBase);

protected:
    /// VisualizationDriver for Warehouse service should be able to directly access ::deck pointer.
    friend class VisualizationDriver;

    explicit ContainerBase (class CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~ContainerBase () noexcept = default;

    class CargoDeck *deck;

    StandardLayout::Mapping typeMapping;
};
} // namespace Emergence::Galleon
