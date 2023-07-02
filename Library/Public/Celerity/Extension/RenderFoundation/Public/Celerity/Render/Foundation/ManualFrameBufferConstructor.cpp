#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/FrameBuffer.hpp>
#include <Celerity/Render/Foundation/FrameBufferTextureUsage.hpp>
#include <Celerity/Render/Foundation/ManualFrameBufferConstructor.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity
{
ManualFrameBufferConstructor::ManualFrameBufferConstructor (TaskConstructor &_constructor) noexcept
    : fetchTexture (FETCH_VALUE_1F (Texture, assetId)),
      insertFrameBuffer (INSERT_LONG_TERM (FrameBuffer)),
      insertFrameBufferTextureUsage (INSERT_LONG_TERM (FrameBufferTextureUsage))
{
}

void ManualFrameBufferConstructor::ConstructManualFrameBuffer (
    ManualAssetConstructor &_assetConstructor,
    Memory::UniqueString _assetId,
    const Container::Vector<Memory::UniqueString> &_textureIds) noexcept
{
    _assetConstructor.ConstructManualAsset (_assetId, FrameBuffer::Reflect ().mapping);
    auto usageCursor = insertFrameBufferTextureUsage.Execute ();

    Render::Backend::FrameBufferBuilder builder;
    builder.Begin ();

    for (Memory::UniqueString textureId : _textureIds)
    {
        if (auto textureCursor = fetchTexture.Execute (&textureId);
            const auto *texture = static_cast<const Texture *> (*textureCursor))
        {
            if (texture->texture.IsValid ())
            {
                builder.AddRenderTarget (texture->texture);
                auto *usage = static_cast<FrameBufferTextureUsage *> (++usageCursor);
                usage->assetId = _assetId;
                usage->textureId = textureId;
            }
            else
            {
                EMERGENCE_LOG (ERROR, "RenderFoundation: Texture \"", textureId, "\" used to create frame buffer \"",
                               _assetId, "\" is invalid.");
            }
        }
        else
        {
            EMERGENCE_LOG (ERROR, "RenderFoundation: Failed to fetch texture \"", textureId,
                           "\" to create frame buffer \"", _assetId, "\".");
        }
    }

    auto cursor = insertFrameBuffer.Execute ();
    auto *frameBuffer = static_cast<FrameBuffer *> (++cursor);
    frameBuffer->assetId = _assetId;
    frameBuffer->frameBuffer = builder.End ();
}
} // namespace Emergence::Celerity
