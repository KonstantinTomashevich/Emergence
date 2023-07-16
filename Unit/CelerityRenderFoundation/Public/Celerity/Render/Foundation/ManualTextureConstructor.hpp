#pragma once

#include <Celerity/Asset/ManualAssetConstructor.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Celerity
{
/// \brief Utility class for systems that need to manually construct texture
///        assets instead of constructing them through references.
/// \details Useful for things like frame buffer render targets that are not being loaded as regular textures.
class ManualTextureConstructor final
{
public:
    /// \brief Initializes internal queries using given task constructor,
    ManualTextureConstructor (TaskConstructor &_constructor) noexcept;

    ManualTextureConstructor (const ManualTextureConstructor &_other) = delete;

    ManualTextureConstructor (ManualTextureConstructor &&_other) = delete;

    ~ManualTextureConstructor () noexcept = default;

    /// \brief Constructs Texture asset with given id and given backend texture instance.
    void ConstructManualTexture (ManualAssetConstructor &_assetConstructor,
                                 Memory::UniqueString _assetId,
                                 Render::Backend::Texture _texture) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ManualTextureConstructor);

private:
    InsertLongTermQuery insertTexture;
};
} // namespace Emergence::Celerity
