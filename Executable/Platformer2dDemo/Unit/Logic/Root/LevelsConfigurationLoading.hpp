#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace LevelsConfigurationLoading
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        Emergence::Resource::Provider::ResourceProvider *_resourceProvider) noexcept;
} // namespace LevelsConfigurationLoading
