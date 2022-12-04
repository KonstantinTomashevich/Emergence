#include <Assert/Assert.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>

namespace Emergence::Celerity
{
void FrameInputAccumulator::SetCurrentQualifiersMask (QualifiersMask _mask) noexcept
{
    qualifiersMask = _mask;
}

QualifiersMask FrameInputAccumulator::GetCurrentQualifiersMask () const noexcept
{
    return qualifiersMask;
}

const Container::Vector<InputEvent> &FrameInputAccumulator::GetEvents () const noexcept
{
    return events;
}

void FrameInputAccumulator::RecordEvent (const InputEvent &_event) noexcept
{
    EMERGENCE_ASSERT (events.empty () || events.back ().timeNs <= _event.timeNs);
    events.emplace_back (_event);
}

void FrameInputAccumulator::Clear () noexcept
{
    events.clear ();
}
} // namespace Emergence::Celerity
