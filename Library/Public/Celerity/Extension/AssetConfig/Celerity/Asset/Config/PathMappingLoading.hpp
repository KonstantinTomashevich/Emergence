#pragma once

#include <Celerity/Asset/Config/TypeMeta.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetConfigPathMappingLoading
{
/// \brief Contains checkpoints, supported by tasks from ::AddToLoadingPipeline.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Path mapping loading routine starts after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Path mapping loading routine is finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Name of binary config path mapping file.
extern const char *const BINARY_FILE_NAME;

/// \brief Name of YAML config path mapping file.
/// \details YAML file is used only if binary file is not present.
extern const char *const YAML_FILE_NAME;

/// \brief Adds task that loads config path mapping and initializes AssetConfigLoadingStateSingleton.
void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Container::String _assetRootPath,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;

/// \brief Defines an item of config path mapping.
struct ListItem final
{
    /// \brief Maximum length of ::folder.
    static constexpr std::size_t RELATIVE_PATH_MAX_LENGTH = 128u;

    /// \brief Name of the config type mapped to this item.
    /// \invariant Type to item relationship is 1 to 1.
    Memory::UniqueString typeName;

    /// \brief Path to folder with configs from asset root.
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
