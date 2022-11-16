#include <bx/math.h>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BGFX/Rendering.hpp>
#include <Celerity/Render2d/Material2d.hpp>
#include <Celerity/Render2d/Material2dInstance.hpp>
#include <Celerity/Render2d/Texture2d.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::BGFX
{
struct Vertex final
{
    Math::Vector2f translation;
    Math::Vector2f uv;
};

static const Vertex QUAD_VERTICES[4u] = {
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f}, {1.0f, 1.0f}},
    {{-1.0f, -1.0f}, {0.0f, 1.0f}},
};

static const uint16_t QUAD_INDICES[6u] = {2u, 1u, 0u, 0u, 3u, 2u};

RenderingBackend::RenderingBackend (TaskConstructor &_constructor) noexcept
    : fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialInstanceById (FETCH_VALUE_1F (Material2dInstance, assetId)),
      fetchMaterialById (FETCH_VALUE_1F (Material2d, assetId)),
      fetchTextureById (FETCH_VALUE_1F (Texture2d, assetId)),

      fetchUniformByAssetIdAndName (FETCH_VALUE_2F (Uniform2d, assetId, name)),
      fetchUniformVector4fByInstanceId (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fByInstanceId (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fByInstanceId (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerByInstanceId (FETCH_VALUE_1F (UniformSamplerValue, assetId))
{
    vertexLayout.begin ()
        .add (bgfx::Attrib::Position, 2u, bgfx::AttribType::Float)
        .add (bgfx::Attrib::TexCoord0, 2u, bgfx::AttribType::Float)
        .end ();
}

void RenderingBackend::SubmitCamera (const Math::Transform2d &_transform,
                                     const Math::Vector2f &_halfOrthographicSize) noexcept
{
    float view[16u];
    bx::mtxSRT (view, _transform.scale.x, _transform.scale.y, 1.0f, 0.0f, 0.0f, _transform.rotation,
                -_transform.translation.x, -_transform.translation.y, 1.0f);

    float projection[16];
    bx::mtxOrtho (projection, -_halfOrthographicSize.x, _halfOrthographicSize.x, -_halfOrthographicSize.y,
                  _halfOrthographicSize.y, 0.5f, 2.0f, 0.0f, bgfx::getCaps ()->homogeneousDepth);
    bgfx::setViewTransform (0, view, projection);
}

void RenderingBackend::SubmitMaterialInstance (Memory::UniqueString _materialInstanceId) noexcept
{
    materialSubmitted = false;
    auto assetCursor = fetchAssetById.Execute (&_materialInstanceId);
    const auto *asset = static_cast<const Asset *> (*assetCursor);

    if (!asset || asset->state != AssetState::READY)
    {
        EMERGENCE_LOG (WARNING, "RenderingBackend: Material instance \"", _materialInstanceId,
                       "\" cannot be submitted as it is not loaded.");
        return;
    }

    auto materialInstanceCursor = fetchMaterialInstanceById.Execute (&_materialInstanceId);
    const auto *materialInstance = static_cast<const Material2dInstance *> (*materialInstanceCursor);
    EMERGENCE_ASSERT (materialInstance);

    auto materialCursor = fetchMaterialById.Execute (&materialInstance->materialId);
    const auto *material = static_cast<const Material2d *> (*materialCursor);
    EMERGENCE_ASSERT (material);
    currentMaterialProgramHandle.idx = static_cast<uint16_t> (material->nativeHandle);

    struct
    {
        Memory::UniqueString assetId;
        Memory::UniqueString name;
    } uniformQuery;

    auto setPlainUniformValue = [this, material, &uniformQuery, _materialInstanceId] (const auto *_value)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = _value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            bgfx::setUniform ({static_cast<uint16_t> (uniform->nativeHandle)}, &_value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "RenderingBackend: Material instance uniform value \"", _materialInstanceId, ".",
                           _value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    };

    for (auto valueCursor = fetchUniformVector4fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformVector4fValue *> (*valueCursor); ++valueCursor)
    {
        setPlainUniformValue (value);
    }

    for (auto valueCursor = fetchUniformMatrix3x3fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformMatrix3x3fValue *> (*valueCursor); ++valueCursor)
    {
        setPlainUniformValue (value);
    }

    for (auto valueCursor = fetchUniformMatrix4x4fByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformMatrix4x4fValue *> (*valueCursor); ++valueCursor)
    {
        setPlainUniformValue (value);
    }

    for (auto valueCursor = fetchUniformSamplerByInstanceId.Execute (&_materialInstanceId);
         const auto *value = static_cast<const UniformSamplerValue *> (*valueCursor); ++valueCursor)
    {
        auto textureAssetCursor = fetchAssetById.Execute (&value->textureId);
        const auto *textureAsset = static_cast<const Asset *> (*textureAssetCursor);

        if (!textureAsset || textureAsset->state != AssetState::READY)
        {
            EMERGENCE_LOG (WARNING, "RenderingBackend: Material instance uniform value \"", _materialInstanceId, ".",
                           value->uniformName,
                           "\" cannot be submitted as required texture is not loaded. Skipping material submit.");
            return;
        }

        auto textureCursor = fetchTextureById.Execute (&value->textureId);
        const auto *texture = static_cast<const Texture2d *> (*textureCursor);
        EMERGENCE_ASSERT (texture);

        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            bgfx::setTexture (value->textureStage, {static_cast<uint16_t> (uniform->nativeHandle)},
                              {static_cast<uint16_t> (texture->nativeHandle)});
        }
        else
        {
            EMERGENCE_LOG (WARNING, "RenderingBackend: Material instance uniform value \"", _materialInstanceId, ".",
                           value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    }

    materialSubmitted = true;
}

void RenderingBackend::SubmitRects (const Container::Vector<RectData> &_rects) noexcept
{
    if (!materialSubmitted)
    {
        EMERGENCE_LOG (WARNING, "RenderingBackend: Rect submit skipped because material is not submitted.");
        return;
    }

    const auto totalVertices = static_cast<uint32_t> (_rects.size () * 4u);
    const auto totalIndices = static_cast<uint32_t> (_rects.size () * 6u);

    const uint32_t availableVertices = bgfx::getAvailTransientVertexBuffer (totalVertices, vertexLayout);
    const uint32_t availableIndices = bgfx::getAvailTransientIndexBuffer (totalIndices, false);
    const uint32_t rectsInBatch = std::min (availableVertices / 4u, availableIndices / 6u);

    for (uint32_t startIndex = 0u; startIndex < _rects.size (); startIndex += rectsInBatch)
    {
        const uint32_t size = std::min (rectsInBatch, static_cast<uint32_t> (_rects.size ()) - startIndex);
        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;

        if (!bgfx::allocTransientBuffers (&vertexBuffer, vertexLayout, size * 4u, &indexBuffer, size * 6u, false))
        {
            EMERGENCE_LOG (WARNING,
                           "RenderingBackend: Unable to finish rect submit due to being unable to allocate buffers.");
            return;
        }

        for (uint32_t index = 0u; index < size; ++index)
        {
            const RectData &rect = _rects[startIndex + index];
            const Math::Matrix3x3f transformMatrix {rect.transform};
            Vertex *vertices = reinterpret_cast<Vertex *> (vertexBuffer.data) + static_cast<ptrdiff_t> (index * 4u);

            for (uint32_t vertexIndex = 0u; vertexIndex < 4u; ++vertexIndex)
            {
                Vertex &vertex = vertices[vertexIndex];
                const Math::Vector2f localPoint = QUAD_VERTICES[vertexIndex].translation * rect.halfSize;
                const Math::Vector3f globalPoint3f =
                    transformMatrix * Math::Vector3f {localPoint.x, localPoint.y, 1.0f};

                vertex.translation.x = globalPoint3f.x;
                vertex.translation.y = globalPoint3f.y;

                vertex.uv.x = rect.uv.min.x + QUAD_VERTICES[vertexIndex].uv.x * (rect.uv.max.x - rect.uv.min.x);
                vertex.uv.y = rect.uv.min.y + QUAD_VERTICES[vertexIndex].uv.y * (rect.uv.max.y - rect.uv.min.y);
            }

            uint16_t *indices = reinterpret_cast<uint16_t *> (indexBuffer.data) + static_cast<ptrdiff_t> (index * 6u);
            for (uint32_t indexIndex = 0u; indexIndex < 6u; ++indexIndex)
            {
                indices[indexIndex] = static_cast<uint16_t> (QUAD_INDICES[indexIndex] + index * 4u);
            }
        }

        bgfx::setState (BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A |
                        BGFX_STATE_BLEND_ALPHA | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA);
        bgfx::setVertexBuffer (0, &vertexBuffer);
        bgfx::setIndexBuffer (&indexBuffer);
        bgfx::submit (0u, currentMaterialProgramHandle);
    }
}

void RenderingBackend::EndFrame () noexcept
{
    bgfx::touch (0);
    bgfx::frame ();
    materialSubmitted = false;
}
} // namespace Emergence::Celerity::BGFX
