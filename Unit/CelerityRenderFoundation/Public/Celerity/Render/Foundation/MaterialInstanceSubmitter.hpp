#pragma once

#include <CelerityRenderFoundationApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <Render/Backend/Renderer.hpp>

namespace Emergence::Celerity
{
/// \brief Utility class for submitting material instance uniform values.
/// \details Encapsulating uniform submission logic makes it easier for users to implement their renderers.
class CelerityRenderFoundationApi MaterialInstanceSubmitter final
{
public:
    /// \brief Initializes internal queries using given task constructor,
    MaterialInstanceSubmitter (TaskConstructor &_constructor) noexcept;

    MaterialInstanceSubmitter (const MaterialInstanceSubmitter &_other) = delete;

    MaterialInstanceSubmitter (MaterialInstanceSubmitter &&_other) = delete;

    ~MaterialInstanceSubmitter () noexcept = default;

    /// \brief Attempts to submit values for all uniforms of material instance.
    /// \return Program id of used material or null option in case of errors.
    Container::Optional<Render::Backend::ProgramId> Submit (Render::Backend::SubmissionAgent &_agent,
                                                            Memory::UniqueString _materialInstanceId) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (MaterialInstanceSubmitter);

private:
    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchMaterialInstanceById;
    FetchValueQuery fetchMaterialById;

    FetchValueQuery fetchUniformVector4fByInstanceId;
    FetchValueQuery fetchUniformMatrix3x3fByInstanceId;
    FetchValueQuery fetchUniformMatrix4x4fByInstanceId;
    FetchValueQuery fetchUniformSamplerByInstanceId;

    FetchValueQuery fetchUniformByAssetIdAndName;
    FetchValueQuery fetchTextureById;
};
} // namespace Emergence::Celerity
