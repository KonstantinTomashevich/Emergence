#include <Assert/Assert.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>

namespace Emergence::Celerity
{
InputEvent::InputEvent (uint64_t _timeNs, const KeyboardEvent &_event) noexcept
    : type (InputEventType::KEYBOARD),
      timeNs (_timeNs),
      keyboard (_event)
{
}

InputEvent::InputEvent (uint64_t _timeNs, const MouseButtonEvent &_event) noexcept
    : type (InputEventType::MOUSE_BUTTON),
      timeNs (_timeNs),
      mouseButton (_event)
{
}

InputEvent::InputEvent (uint64_t _timeNs, const MouseMotionEvent &_event) noexcept
    : type (InputEventType::MOUSE_MOTION),
      timeNs (_timeNs),
      mouseMotion (_event)
{
}

InputEvent::InputEvent (uint64_t _timeNs, const MouseWheelEvent &_event) noexcept
    : type (InputEventType::MOUSE_WHEEL),
      timeNs (_timeNs),
      mouseWheel (_event)
{
}

InputEvent::InputEvent (uint64_t _timeNs, const TextInputEvent &_event) noexcept
    : type (InputEventType::TEXT_INPUT),
      timeNs (_timeNs),
      textInput (_event)
{
}

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
    qualifiersMask = 0u;
}
} // namespace Emergence::Celerity
