#include <Celerity/Asset/Asset.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Material.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/MaterialInstanceSubmitter.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity
{
MaterialInstanceSubmitter::MaterialInstanceSubmitter (TaskConstructor &_constructor) noexcept
    : fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialInstanceById (FETCH_VALUE_1F (MaterialInstance, assetId)),
      fetchMaterialById (FETCH_VALUE_1F (Material, assetId)),

      fetchUniformVector4fByInstanceId (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fByInstanceId (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fByInstanceId (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerByInstanceId (FETCH_VALUE_1F (UniformSamplerValue, assetId)),

      fetchUniformByAssetIdAndName (FETCH_VALUE_2F (Uniform, assetId, name)),
      fetchTextureById (FETCH_VALUE_1F (Texture, assetId))
{
}

Container::Optional<Render::Backend::ProgramId> MaterialInstanceSubmitter::Submit (
    Render::Backend::SubmissionAgent &_agent, Memory::UniqueString _materialInstanceId) noexcept
{
    auto assetCursor = fetchAssetById.Execute (&_materialInstanceId);
    const auto *asset = static_cast<const Asset *> (*assetCursor);

    if (!asset || asset->state != AssetState::READY)
    {
        EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance \"", _materialInstanceId,
                       "\" cannot be submitted as it is not loaded.");
        return std::nullopt;
    }

    auto materialInstanceCursor = fetchMaterialInstanceById.Execute (&_materialInstanceId);
    const auto *materialInstance = static_cast<const MaterialInstance *> (*materialInstanceCursor);
    EMERGENCE_ASSERT (materialInstance);

    auto materialCursor = fetchMaterialById.Execute (&materialInstance->materialId);
    const auto *material = static_cast<const Material *> (*materialCursor);
    EMERGENCE_ASSERT (material);

    struct
    {
        Memory::UniqueString assetId;
        Memory::UniqueString name;
    } uniformQuery;

    for (auto valueCursor = fetchUniformVector4fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformVector4fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform *> (*uniformCursor))
        {
            _agent.SetVector4f (uniform->uniform.GetId (), value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance uniform value \"",
                           _materialInstanceId, ".", value->uniformName,
                           "\" cannot be submitted as it is not registered in material.");
            return std::nullopt;
        }
    }

    for (auto valueCursor = fetchUniformMatrix3x3fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformMatrix3x3fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform *> (*uniformCursor))
        {
            _agent.SetMatrix3x3f (uniform->uniform.GetId (), value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance uniform value \"",
                           _materialInstanceId, ".", value->uniformName,
                           "\" cannot be submitted as it is not registered in material.");
            return std::nullopt;
        }
    }

    for (auto valueCursor = fetchUniformMatrix4x4fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformMatrix4x4fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform *> (*uniformCursor))
        {
            _agent.SetMatrix4x4f (uniform->uniform.GetId (), value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance uniform value \"",
                           _materialInstanceId, ".", value->uniformName,
                           "\" cannot be submitted as it is not registered in material.");
            return std::nullopt;
        }
    }

    for (auto valueCursor = fetchUniformSamplerByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformSamplerValue *> (*valueCursor); ++valueCursor)
    {
        auto textureAssetCursor = fetchAssetById.Execute (&value->textureId);
        const auto *textureAsset = static_cast<const Asset *> (*textureAssetCursor);

        if (!textureAsset || textureAsset->state != AssetState::READY)
        {
            EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance uniform value \"",
                           _materialInstanceId, ".", value->uniformName,
                           "\" cannot be submitted as required texture is not loaded. Skipping material submit.");
            return std::nullopt;
        }

        auto textureCursor = fetchTextureById.Execute (&value->textureId);
        const auto *texture = static_cast<const Texture *> (*textureCursor);
        EMERGENCE_ASSERT (texture);

        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform *> (*uniformCursor))
        {
            _agent.SetSampler (uniform->uniform.GetId (), uniform->textureStage, texture->texture.GetId ());
        }
        else
        {
            EMERGENCE_LOG (WARNING, "MaterialInstanceSubmitter: Material instance uniform value \"",
                           _materialInstanceId, ".", value->uniformName,
                           "\" cannot be submitted as it is not registered in material.");
            return std::nullopt;
        }
    }

    return material->program.GetId ();
}
} // namespace Emergence::Celerity
