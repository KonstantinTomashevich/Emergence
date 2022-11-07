#pragma once

#include <bgfx/bgfx.h>

#include <Celerity/PipelineBuilder.hpp>

#include <Math/AxisAlignedBox2d.hpp>
#include <Math/Transform2d.hpp>

namespace Emergence::Celerity::BGFX
{
struct RectData final
{
    Math::Transform2d transform;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
};

class RenderingBackend final
{
public:
    RenderingBackend (TaskConstructor &_constructor) noexcept;

    static void SubmitCamera (const Math::Transform2d &_transform,
                              const Math::Vector2f &_halfOrthographicSize) noexcept;

    void SubmitMaterialInstance (Memory::UniqueString _materialInstanceId) noexcept;

    void SubmitRects (const Container::Vector<RectData> &_rects) noexcept;

    void EndFrame () noexcept;

private:
    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchMaterialInstanceById;
    FetchValueQuery fetchMaterialById;
    FetchValueQuery fetchTextureById;

    FetchValueQuery fetchUniformByAssetIdAndName;
    FetchValueQuery fetchUniformVector4fByInstanceId;
    FetchValueQuery fetchUniformMatrix3x3fByInstanceId;
    FetchValueQuery fetchUniformMatrix4x4fByInstanceId;
    FetchValueQuery fetchUniformSamplerByInstanceId;

    bool materialSubmitted = false;
    bgfx::ProgramHandle currentMaterialProgramHandle {bgfx::kInvalidHandle};
    bgfx::VertexLayout vertexLayout;
};
} // namespace Emergence::Celerity::BGFX
