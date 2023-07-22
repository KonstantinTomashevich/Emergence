#pragma once

#include <CelerityAssetLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
/// \brief Utility class for systems that need to manually construct
///        assets instead of constructing them through references.
class CelerityAssetLogicApi ManualAssetConstructor final
{
public:
    /// \brief Initializes internal queries using given task constructor,
    ManualAssetConstructor (TaskConstructor &_constructor) noexcept;

    ManualAssetConstructor (const ManualAssetConstructor &_other) = delete;

    ManualAssetConstructor (ManualAssetConstructor &&_other) = delete;

    ~ManualAssetConstructor () noexcept = default;

    /// \brief Constructs ready to use asset with given id, given type and zero references.
    /// \warning Asset might be cleaned up later by asset management system unless something references it.
    void ConstructManualAsset (Memory::UniqueString _assetId, const StandardLayout::Mapping &_assetType) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ManualAssetConstructor);

private:
    ModifySingletonQuery modifyAssetManager;
    InsertLongTermQuery insertAsset;
};
} // namespace Emergence::Celerity
