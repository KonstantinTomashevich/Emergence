#pragma once

#include <Platformer2dDemoBaseApi.hpp>

#include <Celerity/Nexus/Nexus.hpp>

#include <Container/Any.hpp>
#include <Container/Optional.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <VirtualFileSystem/Context.hpp>

using namespace Emergence::Memory::Literals;

struct Platformer2dDemoBaseApi NexusUserContext
{
    NexusUserContext () noexcept;

    NexusUserContext (const NexusUserContext &_other) = delete;

    NexusUserContext (NexusUserContext &&_other) = delete;

    ~NexusUserContext () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (NexusUserContext);

    Emergence::VirtualFileSystem::Context virtualFileSystem;
    Emergence::InputStorage::FrameInputAccumulator inputAccumulator;

    Emergence::VirtualFileSystem::Entry resourcesRoot;
    Emergence::VirtualFileSystem::MountConfigurationList resourcesMount;
    Emergence::Container::Optional<Emergence::Resource::Provider::ResourceProvider> resourceProvider;

    Emergence::Celerity::NexusNode *gameStateNode = nullptr;
    Emergence::Celerity::NexusNode *gameWorldNode = nullptr;

    Emergence::Container::Any assetReferenceBindingList {
        Emergence::Memory::Profiler::AllocationGroup {"NexusUserContext"_us}};
    Emergence::Container::Any assetReferenceBindingEventMap {
        Emergence::Memory::Profiler::AllocationGroup {"NexusUserContext"_us}};

    bool terminateRequested = false;
};
