#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Links frame buffer asset to its used texture.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
struct CelerityRenderFoundationModelApi FrameBufferTextureUsage final
{
    Memory::UniqueString assetId;
    Memory::UniqueString textureId;

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId textureId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
