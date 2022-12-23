#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/AssetFileLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
AssetFileLoadingState::AssetFileLoadingState () noexcept = default;

AssetFileLoadingState::~AssetFileLoadingState () noexcept
{
    if (data)
    {
        EMERGENCE_ASSERT (size > 0u);
        allocator.Release (data, size);
    }

    if (sourceFile)
    {
        fclose (sourceFile);
    }
}

AssetFileLoadingStatus AssetFileLoadingState::StartLoading (const char *_fileName) noexcept
{
    sourceFile = fopen (_fileName, "rb");
    if (!sourceFile)
    {
        return AssetFileLoadingStatus::FAILED;
    }

    fseek (sourceFile, 0u, SEEK_END);
    size = static_cast<uint32_t> (ftell (sourceFile));
    fseek (sourceFile, 0u, SEEK_SET);
    data = static_cast<uint8_t *> (allocator.Acquire (size, alignof (uint8_t)));
    return AssetFileLoadingStatus::LOADING;
}

AssetFileLoadingStatus AssetFileLoadingState::ContinueLoading (uint32_t _chunkSize) noexcept
{
    EMERGENCE_ASSERT (sourceFile);
    EMERGENCE_ASSERT (data);
    const uint32_t toRead = std::min (_chunkSize, size - read);

    if (fread (data + read, 1u, toRead, sourceFile) != toRead)
    {
        return AssetFileLoadingStatus::FAILED;
    }

    read += toRead;
    return read == size ? AssetFileLoadingStatus::FINISHED : AssetFileLoadingStatus::LOADING;
}

const AssetFileLoadingState::Reflection &AssetFileLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetFileLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (data);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTER_REGULAR (read);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
