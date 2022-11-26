#pragma once

#include <bgfx/bgfx.h>

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Render2d/Viewport2d.hpp>

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

    static void UpdateViewportConfiguration (const Viewport2d &_viewport) noexcept;

    static void SubmitCamera (std::uint16_t _nativeViewportId, const Math::Transform2d &_transform,
                              const Math::Vector2f &_halfOrthographicSize) noexcept;

    void SubmitMaterialInstance (Memory::UniqueString _materialInstanceId) noexcept;

    void SubmitRects (std::uint16_t _nativeViewportId, const Container::Vector<RectData> &_rects) noexcept;

    static void TouchView (std::uint16_t _nativeViewportId) noexcept;

    static void SubmitViewOrder (const Container::Vector<std::uint16_t> &_viewOrder) noexcept;

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
