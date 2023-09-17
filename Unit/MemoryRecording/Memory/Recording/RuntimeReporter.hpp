#pragma once

#include <MemoryRecordingApi.hpp>

#include <Container/Optional.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/ReporterBase.hpp>
#include <Memory/Recording/ReportingHelpers.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Reports MemoryProfiler events directly into Track.
class MemoryRecordingApi RuntimeReporter final : public ReporterBase
{
public:
    RuntimeReporter () noexcept = default;

    RuntimeReporter (const RuntimeReporter &_other) = delete;

    RuntimeReporter (RuntimeReporter &&_other) noexcept;

    ~RuntimeReporter () noexcept;

    /// \brief Begin reporting events into given track with given captured state.
    /// \invariant Not reporting into another track already.
    void Begin (Track *_target, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept;

    /// \brief Reports given event to associated track.
    /// \invariant Currently reporting events to some track.
    void ReportEvent (const Profiler::Event &_event) noexcept;

    /// \brief End current event reporting session.
    /// \invariant Currently reporting events to some track.
    void End () noexcept;

    RuntimeReporter &operator= (const RuntimeReporter &_other) = delete;

    RuntimeReporter &operator= (RuntimeReporter &&_other) noexcept;

private:
    GroupUIDAssigner uidAssigner;
};
} // namespace Emergence::Memory::Recording
