#include <cassert>

#include <Memory/Recording/ReporterBase.hpp>
#include <Memory/Recording/Track.hpp>

namespace Emergence::Memory::Recording
{
void ReporterBase::Begin (Track *_track) noexcept
{
    assert (!track);
    assert (_track);
    track = _track;
}

void ReporterBase::ReportEvent (const Event &_event) noexcept
{
    assert (track);
    track->ReportEvent (_event);
}

void ReporterBase::End () noexcept
{
    track = nullptr;
}
} // namespace Emergence::Memory::Recording
