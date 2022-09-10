#pragma once

#include <Celerity/Asset/Config/TypeMeta.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetConfigPathMappingLoading
{
/// \brief Contains checkpoints, supported by tasks from ::AddToLoadingPipeline.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Loading pipeline processing starts after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Loading pipeline processing is finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

extern const char *const BINARY_FILE_NAME;
extern const char *const YAML_FILE_NAME;

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Container::String _assetRootPath,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;

struct ListItem final
{
    static constexpr std::size_t RELATIVE_PATH_MAX_LENGTH = 128u;

    Memory::UniqueString typeName;
    std::array<char, RELATIVE_PATH_MAX_LENGTH> folder;

    struct Reflection final
    {
        StandardLayout::FieldId typeName;
        StandardLayout::FieldId folder;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity::AssetConfigPathMappingLoading
