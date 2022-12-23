#pragma once

#include <cstdio>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates possible return values for AssetFileLoadingState utility IO methods.
enum class AssetFileLoadingStatus
{
    /// \brief Still loading data from file.
    LOADING = 0u,

    /// \brief Failed to load data from file.
    FAILED,

    /// \brief Finished loading data from file.
    FINISHED,
};

/// \brief Utility extension for the assets that are loaded from big files and have load-then-init logic.
/// \details For example, we can use this for textures or fonts: their files are quite big and cannot be loaded in
///          one frame, therefore we're loading them during several frames and then do initialization logic. This
///          extension provides means to save loading-time data.
struct AssetFileLoadingState final
{
    EMERGENCE_STATIONARY_DATA_TYPE (AssetFileLoadingState);

    /// \brief Id of asset, to which this extension is attached during loading.
    Memory::UniqueString assetId;

    /// \brief Allocator for allocation ::data.
    Memory::Heap allocator {Memory::Profiler::AllocationGroup::Top ()};

    /// \brief Handle to opened file if any.
    FILE *sourceFile = nullptr;

    /// \brief Buffer for data loading.
    uint8_t *data = nullptr;

    /// \brief Indicates full size of ::sourceFile in bytes.
    uint32_t size = 0u;

    /// \brief Indicates how many bytes are already loaded.
    uint32_t read = 0u;

    /// \brief Utility method that starts loading routine: opens file, tries to query its size and allocates buffer.
    /// \details Has simple logic that can be used anywhere, therefore moved to structure API.
    AssetFileLoadingStatus StartLoading (const char *_fileName) noexcept;

    /// \brief Utility method that continues loading routine: reads next data chunk from the file.
    /// \details Has simple logic that can be used anywhere, therefore moved to structure API.
    AssetFileLoadingStatus ContinueLoading (uint32_t _chunkSize) noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId data;
        StandardLayout::FieldId size;
        StandardLayout::FieldId read;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
