#include <Celerity/Resource/Object/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
void *ResourceObjectLoadingSharedState::operator new (std::size_t /*unused*/) noexcept
{
    return GetHeap ().Acquire (sizeof (ResourceObjectLoadingSharedState), alignof (ResourceObjectLoadingSharedState));
}

void ResourceObjectLoadingSharedState::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, sizeof (ResourceObjectLoadingSharedState));
}

ResourceObjectLoadingSharedState::ResourceObjectLoadingSharedState (
    Resource::Provider::ResourceProvider *_resourceProvider, Resource::Object::TypeManifest _typeManifest) noexcept
    : libraryLoader (_resourceProvider, std::move (_typeManifest))
{
}

Memory::Profiler::AllocationGroup ResourceObjectLoadingSharedState::GetAllocationGroup () noexcept
{
    using namespace Memory::Literals;
    return Memory::Profiler::AllocationGroup {
        {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "Celerity"_us}, "Shared"_us},
        Memory::UniqueString {"ResourceObjectLoading"}};
}

Memory::Heap &ResourceObjectLoadingSharedState::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}

const ResourceObjectLoadingStateSingleton::Reflection &ResourceObjectLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
