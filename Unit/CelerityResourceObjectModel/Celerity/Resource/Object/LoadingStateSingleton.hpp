#pragma once

#include <CelerityResourceObjectModelApi.hpp>

#include <Container/HashSet.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Resource/Object/LibraryLoader.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains resource object library loading state that is shared with background job.
class CelerityResourceObjectModelApi ResourceObjectLoadingSharedState final : public Handling::HandleableBase
{
public:
    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    ResourceObjectLoadingSharedState (Resource::Provider::ResourceProvider *_resourceProvider,
                                      Resource::Object::TypeManifest _typeManifest) noexcept;

    ResourceObjectLoadingSharedState (const ResourceObjectLoadingSharedState &_other) = delete;

    ResourceObjectLoadingSharedState (ResourceObjectLoadingSharedState &&_other) = delete;

    ~ResourceObjectLoadingSharedState () noexcept = default;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceObjectLoadingSharedState);

    /// \brief Whether loading is finished.
    std::atomic_flag loaded;

    /// \brief Library loading state.
    Resource::Object::LibraryLoader libraryLoader;

    /// \brief Library loading output.
    Resource::Object::Library library;

    /// \brief List of objects that were requested to be loaded.
    /// \details Used to avoid requesting the same object several times.
    Container::Vector<Memory::UniqueString> requestedObjectList {GetAllocationGroup ()};

private:
    /// \brief Allocation group used by ::GetHeap.
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Heap for allocating instances of this state.
    static Memory::Heap &GetHeap () noexcept;
};

/// \brief Contains loading states for resource object loading requests.
/// \details Is not designed to be modified from outside.
struct CelerityResourceObjectModelApi ResourceObjectLoadingStateSingleton final
{
    /// \brief Loading states of all active requests.
    Container::Vector<Handling::Handle<ResourceObjectLoadingSharedState>> sharedStates {
        Memory::Profiler::AllocationGroup::Top ()};

    struct CelerityResourceObjectModelApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
