#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Shared constants of MemoryRecording library.
class Constants final
{
public:
    Constants () = delete;

    /// \brief All memory recording allocators are using this group to log memory usage.
    static const Profiler::AllocationGroup &AllocationGroup () noexcept;

    /// \brief Marker, added to the end of capture setup events.
    /// \details Captured groups state is saved as sequence of events at the begging of a track.
    ///          This marker indicates end of this sequence.
    static UniqueString CaptureInitializationFinishedMarker () noexcept;
};
} // namespace Emergence::Memory::Recording
