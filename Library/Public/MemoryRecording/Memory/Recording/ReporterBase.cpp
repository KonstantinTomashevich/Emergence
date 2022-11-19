#include <Assert/Assert.hpp>

#include <Memory/Recording/ReporterBase.hpp>
#include <Memory/Recording/Track.hpp>

namespace Emergence::Memory::Recording
{
void ReporterBase::Begin (Track *_track) noexcept
{
    EMERGENCE_ASSERT (!track);
    EMERGENCE_ASSERT (_track);
    track = _track;
}

void ReporterBase::ReportEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (track);
    track->ReportEvent (_event);
}

void ReporterBase::End () noexcept
{
    track = nullptr;
}
} // namespace Emergence::Memory::Recording
