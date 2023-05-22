#include <Celerity/Locale/LocaleSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
void *LocaleLoadingSharedState::operator new (std::size_t /*unused*/) noexcept
{
    return GetHeap ().Acquire (sizeof (LocaleLoadingSharedState), alignof (LocaleLoadingSharedState));
}

void LocaleLoadingSharedState::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, sizeof (LocaleLoadingSharedState));
}

Memory::Profiler::AllocationGroup LocaleLoadingSharedState::GetAllocationGroup () noexcept
{
    using namespace Memory::Literals;
    return Memory::Profiler::AllocationGroup {
        {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "Celerity"_us}, "Shared"_us},
        Memory::UniqueString {"LocaleLoading"}};
}

Memory::Heap &LocaleLoadingSharedState::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}

const LocaleSingleton::Reflection &LocaleSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LocaleSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (targetLocale);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loadedLocale);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loadingLocale);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
