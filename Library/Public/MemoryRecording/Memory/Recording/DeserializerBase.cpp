#include <cassert>

#include <Memory/Recording/DeserializerBase.hpp>
#include <Memory/Recording/Recording.hpp>

namespace Emergence::Memory::Recording
{
void DeserializerBase::Begin (Recording *_recording) noexcept
{
    assert (_recording);
    recording = _recording;
}

void DeserializerBase::ReportEvent (const Event &_event) noexcept
{
    assert (recording);
    recording->ReportEvent (_event);
}

void DeserializerBase::End () noexcept
{
    recording = nullptr;
}
} // namespace Emergence::Memory::Recording
