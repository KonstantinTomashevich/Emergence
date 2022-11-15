#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>

#include <bgfx/bgfx.h>

#include <bimg/decode.h>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render2d/Texture2d.hpp>
#include <Celerity/Asset/Render2d/Texture2dManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BGFX/Allocator.hpp>
#include <Celerity/Render2d/Texture2d.hpp>

#include <Log/Log.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::Texture2dManagement
{
class Manager : public TaskExecutorBase<Manager>
{
public:
    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_textureRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    void ProcessLoading () noexcept;

    AssetState StartLoading (Memory::UniqueString _assetId) noexcept;

    static AssetState ContinueLoadingData (Texture2dLoadingState *_loadingState) noexcept;

    AssetState FinishLoading (Texture2dLoadingState *_loadingState) noexcept;

    void ProcessUnloading () noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertTexture;
    InsertLongTermQuery insertTextureLoadingState;
    RemoveValueQuery removeTextureById;
    ModifyValueQuery modifyTextureLoadingStateById;

    Container::Vector<Memory::UniqueString> textureRootPaths {Memory::Profiler::AllocationGroup::Top ()};
    const uint64_t maxLoadingTimePerFrameNs;
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_textureRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertTexture (INSERT_LONG_TERM (Texture2d)),
      insertTextureLoadingState (INSERT_LONG_TERM (Texture2dLoadingState)),
      removeTextureById (REMOVE_VALUE_1F (Texture2d, assetId)),
      modifyTextureLoadingStateById (MODIFY_VALUE_1F (Texture2dLoadingState, assetId)),

      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
{
    textureRootPaths.reserve (_textureRootPaths.size ());
    for (Memory::UniqueString root : _textureRootPaths)
    {
        textureRootPaths.emplace_back (root);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

void Manager::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

void Manager::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Texture2d::Reflect ().mapping;
        AssetState state = AssetState::LOADING;
    } loadingMaterialsParameter;

    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    for (auto assetCursor = fetchAssetByTypeNumberAndState.Execute (&loadingMaterialsParameter);
         const auto *asset = static_cast<const Asset *> (*assetCursor); ++assetCursor)
    {
        if (Emergence::Time::NanosecondsSinceStartup () - startTime > maxLoadingTimePerFrameNs)
        {
            break;
        }

        AssetState newState = AssetState::LOADING;
        bool needsInitialization;

        {
            auto loadingStateCursor = modifyTextureLoadingStateById.Execute (&asset->id);
            needsInitialization = !*loadingStateCursor;
        }

        if (needsInitialization)
        {
            Unload (asset->id);
            newState = StartLoading (asset->id);
        }

        auto loadingStateCursor = modifyTextureLoadingStateById.Execute (&asset->id);
        auto *loadingState = static_cast<Texture2dLoadingState *> (*loadingStateCursor);

        while (newState == AssetState::LOADING &&
               Emergence::Time::NanosecondsSinceStartup () - startTime < maxLoadingTimePerFrameNs)
        {
            newState = ContinueLoadingData (loadingState);
        }

        if (newState != AssetState::LOADING)
        {
            if (newState == AssetState::READY)
            {
                newState = FinishLoading (loadingState);
            }

            auto eventCursor = insertAssetStateEvent.Execute ();
            auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
            event->assetId = asset->id;
            event->state = newState;

            if (loadingState)
            {
                ~loadingStateCursor;
            }
        }
    }
}

AssetState Manager::StartLoading (Memory::UniqueString _assetId) noexcept
{
    for (Memory::UniqueString root : textureRootPaths)
    {
        const std::filesystem::path path = EMERGENCE_BUILD_STRING (root, "/", _assetId);
        if (std::filesystem::exists (path))
        {
            Container::String pathString = path.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
            auto cursor = insertTextureLoadingState.Execute ();
            auto *state = static_cast<Texture2dLoadingState *> (++cursor);
            state->assetId = _assetId;
            state->sourceFile = fopen (pathString.c_str (), "rb");

            if (!state->sourceFile)
            {
                EMERGENCE_LOG (ERROR, "Texture2dManagement: Unable to open texture file \"", pathString, "\".");
                return AssetState::CORRUPTED;
            }

            fseek (state->sourceFile, 0u, SEEK_END);
            state->size = static_cast<uint32_t> (ftell (state->sourceFile));
            fseek (state->sourceFile, 0u, SEEK_SET);
            state->data = static_cast<uint8_t *> (state->allocator.Acquire (state->size, alignof (uint8_t)));
            return AssetState::LOADING;
        }
    }

    return AssetState::MISSING;
}

AssetState Manager::ContinueLoadingData (Texture2dLoadingState *_loadingState) noexcept
{
    static constexpr uint32_t CHUNK_SIZE = 16u * 1024u;
    const uint32_t toRead = std::min (CHUNK_SIZE, _loadingState->size - _loadingState->read);

    if (fread (_loadingState->data + _loadingState->read, 1u, toRead, _loadingState->sourceFile) != toRead)
    {
        EMERGENCE_LOG (ERROR, "Texture2dManagement: Encountered IO error while reading texture \"",
                       _loadingState->assetId, "\" data.");
        return AssetState::CORRUPTED;
    }

    _loadingState->read += toRead;
    return _loadingState->read == _loadingState->size ? AssetState::READY : AssetState::LOADING;
}

static void ImageReleaseCallback (void * /*unused*/, void *_userData)
{
    bimg::imageFree (static_cast<bimg::ImageContainer *> (_userData));
}

AssetState Manager::FinishLoading (Texture2dLoadingState *_loadingState) noexcept
{
    bimg::ImageContainer *imageContainer =
        bimg::imageParse (BGFX::GetCurrentAllocator (), _loadingState->data, _loadingState->size);

    if (!imageContainer)
    {
        EMERGENCE_LOG (ERROR, "Texture2dManagement: Unable to parse texture data for texture \"",
                       _loadingState->assetId, "\".");
        return AssetState::CORRUPTED;
    }

    // We expect simple 2d texture, not something complex.
    EMERGENCE_ASSERT (!imageContainer->m_cubeMap);
    EMERGENCE_ASSERT (imageContainer->m_depth == 1u);

    if (!bgfx::isTextureValid (0, false, imageContainer->m_numLayers,
                               static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format),
                               BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE))
    {
        EMERGENCE_LOG (ERROR, "Texture2dManagement: Unable to parse texture \"", _loadingState->assetId,
                       "\", because it's data is invalid.");
        return AssetState::CORRUPTED;
    }

    const bgfx::Memory *memory =
        bgfx::makeRef (imageContainer->m_data, imageContainer->m_size, ImageReleaseCallback, imageContainer);

    bgfx::TextureHandle handle = bgfx::createTexture2D (
        static_cast<uint16_t> (imageContainer->m_width), static_cast<uint16_t> (imageContainer->m_height),
        1u < imageContainer->m_numMips, imageContainer->m_numLayers,
        static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
        memory);

    if (!bgfx::isValid (handle))
    {
        EMERGENCE_LOG (ERROR, "Texture2dManagement: Failed to create texture \"", _loadingState->assetId, "\".");
        return AssetState::CORRUPTED;
    }

    auto insertTextureCursor = insertTexture.Execute ();
    auto *texture = static_cast<Texture2d *> (++insertTextureCursor);
    texture->assetId = _loadingState->assetId;
    texture->nativeHandle = static_cast<uint64_t> (handle.idx);
    return AssetState::READY;
}

void Manager::ProcessUnloading () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        Unload (event->id);
    }
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto textureLoadingStateCursor = modifyTextureLoadingStateById.Execute (&_assetId); *textureLoadingStateCursor)
    {
        ~textureLoadingStateCursor;
    }

    if (auto textureCursor = removeTextureById.Execute (&_assetId); textureCursor.ReadConst ())
    {
        ~textureCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_textureRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Texture2d::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "Material2dManagement: Task not registered, because Texture2d is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    _pipelineBuilder.AddTask (Memory::UniqueString {"Texture2dManager"})
        .SetExecutor<Manager> (_textureRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::Texture2dManagement
