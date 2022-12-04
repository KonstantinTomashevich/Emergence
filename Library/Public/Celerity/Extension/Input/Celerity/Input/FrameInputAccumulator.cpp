#include <Assert/Assert.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>

namespace Emergence::Celerity
{
const InputEvent &FrameInputAccumulator::EventIterator::operator* () const noexcept
{
    EMERGENCE_ASSERT (eventsVector && baseIterator != eventsVector->end ());
    return *baseIterator;
}

FrameInputAccumulator::EventIterator &FrameInputAccumulator::EventIterator::operator++ () noexcept
{
    ++baseIterator;
    return *this;
}

bool FrameInputAccumulator::EventIterator::operator== (
    const FrameInputAccumulator::EventIterator &_other) const noexcept
{
    return baseIterator == _other.baseIterator;
}

bool FrameInputAccumulator::EventIterator::operator!= (
    const FrameInputAccumulator::EventIterator &_other) const noexcept
{
    return !(*this == _other);
}

FrameInputAccumulator::EventIterator &FrameInputAccumulator::EventIterator::operator~() noexcept
{
    EMERGENCE_ASSERT (eventsVector);
    baseIterator = eventsVector->erase (baseIterator);
    return *this;
}

FrameInputAccumulator::EventIterator::EventIterator (Container::Vector<InputEvent> *_eventsVector,
                                                     Container::Vector<InputEvent>::iterator _baseIterator) noexcept
    : eventsVector (_eventsVector),
      baseIterator (_baseIterator)
{
    EMERGENCE_ASSERT (eventsVector);
}

void FrameInputAccumulator::SetCurrentQualifiersMask (QualifiersMask _mask) noexcept
{
    qualifiersMask = _mask;
}

QualifiersMask FrameInputAccumulator::GetCurrentQualifiersMask () const noexcept
{
    return qualifiersMask;
}

FrameInputAccumulator::EventIterator FrameInputAccumulator::EventsBegin () noexcept
{
    return {&events, events.begin ()};
}

FrameInputAccumulator::EventIterator FrameInputAccumulator::EventsEnd () noexcept
{
    return {&events, events.end ()};
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
