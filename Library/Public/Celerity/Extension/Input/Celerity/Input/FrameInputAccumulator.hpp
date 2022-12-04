#pragma once

#include <Celerity/Input/Keyboard.hpp>
#include <Celerity/Input/Mouse.hpp>

#include <Container/Vector.hpp>

namespace Emergence::Celerity
{
enum class InputEventType
{
    KEYBOARD,
    MOUSE_BUTTON,
    MOUSE_MOTION,
    MOUSE_WHEEL,
};

struct KeyboardEvent final
{
    KeyCode keyCode = 0u;
    ScanCode scanCode = 0u;
    QualifiersMask qualifiersMask = 0u;
    KeyState keyState = KeyState::DOWN;
    bool repeat = false;
};

// TODO: Support for multiple mouses?

struct MouseButtonEvent final
{
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    MouseButton button = MouseButton::LEFT;
    KeyState state = KeyState::DOWN;
    uint8_t clicks = 0;
};

struct MouseMotionEvent final
{
    int32_t oldX = 0;
    int32_t oldY = 0;
    int32_t newX = 0;
    int32_t newY = 0;
};

struct MouseWheelEvent final
{
    float x = 0.0f;
    float y = 0.0f;
};

struct InputEvent final
{
    InputEventType type;
    uint64_t timeNs = 0u;

    union
    {
        KeyboardEvent keyboard;
        MouseButtonEvent mouseButton;
        MouseMotionEvent mouseMotion;
        MouseWheelEvent mouseWheel;
    };
};

class FrameInputAccumulator final
{
public:
    class EventIterator final
    {
    public:
        EventIterator (const EventIterator &_other) noexcept = default;

        EventIterator (EventIterator &&_other) noexcept = default;

        ~EventIterator () noexcept = default;

        [[nodiscard]] const InputEvent &operator* () const noexcept;

        EventIterator &operator++ () noexcept;

        bool operator== (const EventIterator &_other) const noexcept;

        bool operator!= (const EventIterator &_other) const noexcept;

        EventIterator &operator= (const EventIterator &_other) noexcept = default;

        EventIterator &operator= (EventIterator &&_other) noexcept = default;

        /// \warning Invalidates other iterators!
        EventIterator &operator~() noexcept;

    private:
        friend class FrameInputAccumulator;

        EventIterator (Container::Vector<InputEvent> *_eventsVector,
                       Container::Vector<InputEvent>::iterator _baseIterator) noexcept;

        Container::Vector<InputEvent> *eventsVector;
        Container::Vector<InputEvent>::iterator baseIterator;
    };

    void SetCurrentQualifiersMask (QualifiersMask _mask) noexcept;

    [[nodiscard]] QualifiersMask GetCurrentQualifiersMask () const noexcept;

    [[nodiscard]] EventIterator EventsBegin () noexcept;

    [[nodiscard]] EventIterator EventsEnd () noexcept;

    void RecordEvent (const InputEvent &_event) noexcept;

    void Clear () noexcept;

private:
    QualifiersMask qualifiersMask = 0u;
    Container::Vector<InputEvent> events {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"FrameInputAccumulator"}}};
};
} // namespace Emergence::Celerity
