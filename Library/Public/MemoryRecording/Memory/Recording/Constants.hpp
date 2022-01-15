#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory::Recording
{
class Constants final
{
public:
    Constants () = delete;

    static const Profiler::AllocationGroup &AllocationGroup () noexcept;

    static UniqueString RecordingInitializationFinishedMarker () noexcept;
};
} // namespace Emergence::Memory::Recording
