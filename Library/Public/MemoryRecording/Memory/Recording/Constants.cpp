#include <Memory/Recording/Constants.hpp>

namespace Emergence::Memory::Recording
{
const Profiler::AllocationGroup &Constants::AllocationGroup () noexcept
{
    static Profiler::AllocationGroup group {Profiler::AllocationGroup::Root (), UniqueString {"MemoryRecording"}};
    return group;
}

UniqueString Constants::RecordingInitializationFinishedMarker () noexcept
{
    static UniqueString markerId {"RecordingInitializationFinished"};
    return markerId;
}
} // namespace Emergence::Memory::Recording
