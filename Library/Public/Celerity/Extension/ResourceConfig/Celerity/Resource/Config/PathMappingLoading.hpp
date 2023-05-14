#pragma once

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Resource/Config/TypeMeta.hpp>

namespace Emergence::Celerity::ResourceConfigPathMappingLoading
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

/// \brief Adds task that loads config path mapping and initializes ResourceConfigLoadingStateSingleton.
void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Container::String _resourceRootPath,
                           const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept;

/// \brief Defines an item of config path mapping.
struct ListItem final
{
    /// \brief Name of the config type mapped to this item.
    /// \invariant Type to item relationship is 1 to 1.
    Memory::UniqueString typeName;

    /// \brief Path to folder with configs from resource root.
    Container::Utf8String folder;

    struct Reflection final
    {
        StandardLayout::FieldId typeName;
        StandardLayout::FieldId folder;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Defines structure of config path mapping file.
struct PathMapping final
{
    /// \brief List of all config paths.
    Container::Vector<ListItem> configs {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"ResourceConfigPathMapping"}}};

    struct Reflection final
    {
        StandardLayout::FieldId configs;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity::ResourceConfigPathMappingLoading
