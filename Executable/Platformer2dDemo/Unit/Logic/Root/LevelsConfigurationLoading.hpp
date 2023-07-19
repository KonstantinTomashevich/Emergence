#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace LevelsConfigurationLoading
{
struct Platformer2dDemoLogicApi Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

Platformer2dDemoLogicApi void AddToNormalUpdate (
    Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
    Emergence::Resource::Provider::ResourceProvider *_resourceProvider) noexcept;
} // namespace LevelsConfigurationLoading
