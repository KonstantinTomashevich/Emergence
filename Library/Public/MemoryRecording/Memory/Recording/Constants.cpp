#include <Memory/Recording/Constants.hpp>

namespace Emergence::Memory::Recording
{
const Profiler::AllocationGroup &Constants::AllocationGroup () noexcept
{
    static Profiler::AllocationGroup group {Profiler::AllocationGroup::Root (), UniqueString {"MemoryRecording"}};
    return group;
}

UniqueString Constants::CaptureInitializationFinishedMarker () noexcept
{
    static UniqueString markerId {"CaptureInitializationFinished"};
    return markerId;
}
} // namespace Emergence::Memory::Recording
