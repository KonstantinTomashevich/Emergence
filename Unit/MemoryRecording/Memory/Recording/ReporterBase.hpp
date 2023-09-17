#pragma once

#include <MemoryRecordingApi.hpp>

#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
class Track;

/// \brief Provides inheritors with ability to report events into Track.
class MemoryRecordingApi ReporterBase
{
protected:
    ReporterBase () noexcept = default;

    void Begin (Track *_track) noexcept;

    void ReportEvent (const Event &_event) noexcept;

    void End () noexcept;

private:
    Track *track = nullptr;
};
} // namespace Emergence::Memory::Recording
