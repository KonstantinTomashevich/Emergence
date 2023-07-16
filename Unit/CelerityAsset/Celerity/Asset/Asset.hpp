#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
// TODO: Implement common solution for asset pinning.

/// \brief Describes state of an asset.
enum class AssetState : std::uint8_t
{
    /// \brief Asset is currently loading.
    LOADING,

    /// \brief Requested asset is missing and therefore cannot be loaded.
    MISSING,

    /// \brief Requested asset data is corrupted and therefore asset cannot be loaded.
    CORRUPTED,

    /// \brief Asset is loaded and ready for use.
    READY,
};

/// \brief Contains generic data about referenced asset.
/// \details Instance is automatically created when reference to nonexistent asset is found.
///          If AssetManagerSingleton::automaticallyCleanUnusedAssets is enabled, instances
///          with no references will be automatically removed.
///
///          Asset usage tips:
///          - Attach loaded data by creating objects with the same ::id.
///          - Do not modify Asset instances at all. If you need to update state, use
///            appropriate event from AssetReferenceBindingEventMap::stateUpdate.
///          - Use value query on ::type and ::state pair to request assets that need to
///            be loaded by appropriate loader.
struct Asset final
{
    /// \brief Unique id of this asset.
    Memory::UniqueString id;

    /// \brief Type of this asset used to guide asset loaders.
    StandardLayout::Mapping type;

    /// \brief Current state of this asset.
    /// \details Should be modified only through event from AssetReferenceBindingEventMap::stateUpdate.
    AssetState state = AssetState::LOADING;

    /// \brief Number of references that point to this asset.
    std::uint32_t usages = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId id;

        /// \brief ::type is represented as number so it can be used in indices.
        StandardLayout::FieldId typeNumber;

        StandardLayout::FieldId state;
        StandardLayout::FieldId usages;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
