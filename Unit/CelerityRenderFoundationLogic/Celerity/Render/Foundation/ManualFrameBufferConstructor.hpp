#pragma once

#include <Celerity/Asset/ManualAssetConstructor.hpp>
#include <CelerityRenderFoundationLogicApi.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Celerity
{
/// \brief Utility class for systems that need to construct frame buffer for rendering.
/// \warning Frame buffers can only be constructed through this class and cannot be constructed through reference!
class CelerityRenderFoundationLogicApi ManualFrameBufferConstructor final
{
public:
    /// \brief Initializes internal queries using given task constructor,
    ManualFrameBufferConstructor (TaskConstructor &_constructor) noexcept;

    ManualFrameBufferConstructor (const ManualFrameBufferConstructor &_other) = delete;

    ManualFrameBufferConstructor (ManualFrameBufferConstructor &&_other) = delete;

    ~ManualFrameBufferConstructor () noexcept = default;

    /// \brief Constructs FrameBuffer asset that has given id and uses given textures as outputs.
    void ConstructManualFrameBuffer (ManualAssetConstructor &_assetConstructor,
                                     Memory::UniqueString _assetId,
                                     const Container::Vector<Memory::UniqueString> &_textureIds) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ManualFrameBufferConstructor);

private:
    FetchValueQuery fetchTexture;
    InsertLongTermQuery insertFrameBuffer;
    InsertLongTermQuery insertFrameBufferTextureUsage;
};
} // namespace Emergence::Celerity
