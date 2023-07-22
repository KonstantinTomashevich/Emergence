#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Assert/Assert.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

namespace Emergence::InputStorage
{
bool TextInputEvent::operator== (const TextInputEvent &_other) const noexcept
{
    return strcmp (utf8Value.data (), _other.utf8Value.data ()) == 0;
}

bool TextInputEvent::operator!= (const TextInputEvent &_other) const noexcept
{
    return !(*this == _other);
}

InputEvent::InputEvent (std::uint64_t _timeNs, const KeyboardEvent &_event) noexcept
    : type (InputEventType::KEYBOARD),
      timeNs (_timeNs),
      keyboard (_event)
{
}

InputEvent::InputEvent (std::uint64_t _timeNs, const MouseButtonEvent &_event) noexcept
    : type (InputEventType::MOUSE_BUTTON),
      timeNs (_timeNs),
      mouseButton (_event)
{
}

InputEvent::InputEvent (std::uint64_t _timeNs, const MouseMotionEvent &_event) noexcept
    : type (InputEventType::MOUSE_MOTION),
      timeNs (_timeNs),
      mouseMotion (_event)
{
}

InputEvent::InputEvent (std::uint64_t _timeNs, const MouseWheelEvent &_event) noexcept
    : type (InputEventType::MOUSE_WHEEL),
      timeNs (_timeNs),
      mouseWheel (_event)
{
}

InputEvent::InputEvent (std::uint64_t _timeNs, const TextInputEvent &_event) noexcept
    : type (InputEventType::TEXT_INPUT),
      timeNs (_timeNs),
      textInput (_event)
{
}

bool InputEvent::operator== (const InputEvent &_other) const noexcept
{
    if (type != _other.type || timeNs != _other.timeNs)
    {
        return false;
    }

    switch (type)
    {
    case InputEventType::KEYBOARD:
        return keyboard == _other.keyboard;

    case InputEventType::MOUSE_BUTTON:
        return mouseButton == _other.mouseButton;

    case InputEventType::MOUSE_MOTION:
        return mouseMotion == _other.mouseMotion;

    case InputEventType::MOUSE_WHEEL:
        return mouseWheel == _other.mouseWheel;

    case InputEventType::TEXT_INPUT:
        return textInput == _other.textInput;
    }

    return false;
}

bool InputEvent::operator!= (const InputEvent &_other) const noexcept
{
    return !(*this == _other);
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
    auto place = std::upper_bound (events.begin (), events.end (), _event,
                                   [] (const InputEvent &_first, const InputEvent &_second)
                                   {
                                       return _first.timeNs < _second.timeNs;
                                   });
    events.emplace (place, _event);
}

void FrameInputAccumulator::Clear () noexcept
{
    events.clear ();
    qualifiersMask = 0u;
}
} // namespace Emergence::InputStorage
