#include <Celerity/Resource/Config/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
void *ResourceConfigLoadingSharedState::operator new (std::size_t /*unused*/) noexcept
{
    return GetHeap ().Acquire (sizeof (ResourceConfigLoadingSharedState), alignof (ResourceConfigLoadingSharedState));
}

void ResourceConfigLoadingSharedState::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, sizeof (ResourceConfigLoadingSharedState));
}

ResourceConfigLoadingSharedState::ResourceConfigLoadingSharedState (StandardLayout::Mapping _configType) noexcept
    : configType (std::move (_configType)),
      configPool (GetAllocationGroup (), configType.GetObjectSize (), configType.GetObjectAlignment ())
{
}

ResourceConfigLoadingSharedState::~ResourceConfigLoadingSharedState () noexcept
{
    for (void *config : configPool)
    {
        configType.Destruct (config);
    }
}

Memory::Profiler::AllocationGroup ResourceConfigLoadingSharedState::GetAllocationGroup () noexcept
{
    using namespace Memory::Literals;
    return Memory::Profiler::AllocationGroup {
        {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "Celerity"_us}, "Shared"_us},
        Memory::UniqueString {"ResourceConfigLoading"}};
}

Memory::Heap &ResourceConfigLoadingSharedState::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}

const ResourceConfigLoadingStateSingleton::Reflection &ResourceConfigLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceConfigLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
