#pragma once

#include <InputStorageApi.hpp>

#include <InputStorage/Keyboard.hpp>
#include <InputStorage/Mouse.hpp>

#include <Container/Vector.hpp>

namespace Emergence::InputStorage
{
/// \brief Specifies what kind of event is stored in InputEvent.
enum class InputEventType
{
    KEYBOARD,
    MOUSE_BUTTON,
    MOUSE_MOTION,
    MOUSE_WHEEL,
    TEXT_INPUT,
};

/// \brief Stores information about keyboard input event.
struct InputStorageApi KeyboardEvent final
{
    /// \brief Code of the key, localized to layout.
    KeyCode keyCode = 0u;

    /// \brief Layout-independent key code.
    ScanCode scanCode = 0u;

    /// \brief State of qualifier keys at the time of this event.
    QualifiersMask qualifiersMask = 0u;

    /// \brief New state of the key affected by this event.
    KeyState keyState = KeyState::DOWN;

    bool operator== (const KeyboardEvent &_other) const noexcept = default;

    bool operator!= (const KeyboardEvent &_other) const noexcept = default;
};

// TODO: Support for multiple mouses?

/// \brief Stores information about mouse buttons related event.
struct InputStorageApi MouseButtonEvent final
{
    /// \brief Mouse X coordinate at the time of click, relative to the window.
    int32_t mouseX = 0;

    /// \brief Mouse Y coordinate at the time of click, relative to the window.
    int32_t mouseY = 0;

    /// \brief Button with which event happened.
    MouseButton button = MouseButton::LEFT;

    /// \brief New button state.
    KeyState state = KeyState::DOWN;

    /// \brief How many clicks happened?
    /// \details Useful for catching double-clicks.
    std::uint8_t clicks = 0;

    bool operator== (const MouseButtonEvent &_other) const noexcept = default;

    bool operator!= (const MouseButtonEvent &_other) const noexcept = default;
};

/// \brief Stores information about mouse motion related event.
struct InputStorageApi MouseMotionEvent final
{
    /// \brief Mouse X coordinate before motion happened, relative to the window.
    int32_t oldX = 0;

    /// \brief Mouse Y coordinate before motion happened, relative to the window.
    int32_t oldY = 0;

    /// \brief Mouse X coordinate after motion happened, relative to the window.
    int32_t newX = 0;

    /// \brief Mouse Y coordinate after motion happened, relative to the window.
    int32_t newY = 0;

    bool operator== (const MouseMotionEvent &_other) const noexcept = default;

    bool operator!= (const MouseMotionEvent &_other) const noexcept = default;
};

/// \brief Stores information about mouse wheel related event.
struct InputStorageApi MouseWheelEvent final
{
    /// \brief Horizontal wheel movement for mouses that support it.
    float x = 0.0f;

    /// \brief Vertical wheel movement.
    float y = 0.0f;

    bool operator== (const MouseWheelEvent &_other) const noexcept = default;

    bool operator!= (const MouseWheelEvent &_other) const noexcept = default;
};

/// \brief Stores information about text input from keyboard or other sources (like Android keyboard widget).
struct InputStorageApi TextInputEvent final
{
    /// \brief Maximum amount of bytes per one input event.
    static constexpr std::size_t TEXT_BUFFER_SIZE = 32u;

    /// \brief Input text encoded in utf8 format with zero terminator.
    std::array<char, TEXT_BUFFER_SIZE> utf8Value {"\0"};

    bool operator== (const TextInputEvent &_other) const noexcept;

    bool operator!= (const TextInputEvent &_other) const noexcept;
};

/// \brief Universal structure for storing information about physical input.
struct InputStorageApi InputEvent final
{
    InputEvent (std::uint64_t _timeNs, const KeyboardEvent &_event) noexcept;

    InputEvent (std::uint64_t _timeNs, const MouseButtonEvent &_event) noexcept;

    InputEvent (std::uint64_t _timeNs, const MouseMotionEvent &_event) noexcept;

    InputEvent (std::uint64_t _timeNs, const MouseWheelEvent &_event) noexcept;

    InputEvent (std::uint64_t _timeNs, const TextInputEvent &_event) noexcept;

    /// \brief Type of event that is stored inside.
    InputEventType type;

    /// \brief Time since program startup to the event in nanoseconds.
    std::uint64_t timeNs = 0u;

    union
    {
        KeyboardEvent keyboard;
        MouseButtonEvent mouseButton;
        MouseMotionEvent mouseMotion;
        MouseWheelEvent mouseWheel;
        TextInputEvent textInput;
    };

    bool operator== (const InputEvent &_other) const noexcept;

    bool operator!= (const InputEvent &_other) const noexcept;
};

/// \brief Storage for input events that happened during last frame and need to be processed.
class InputStorageApi FrameInputAccumulator final
{
public:
    /// \brief Provides API for iterating over input events and dropping out consumed events.
    class InputStorageApi EventIterator final
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

        /// \brief Drops out consumed input event so other readers will not see it later.
        /// \warning Invalidates other iterators!
        EventIterator &operator~() noexcept;

    private:
        friend class FrameInputAccumulator;

        EventIterator (Container::Vector<InputEvent> *_eventsVector,
                       Container::Vector<InputEvent>::iterator _baseIterator) noexcept;

        Container::Vector<InputEvent> *eventsVector;
        Container::Vector<InputEvent>::iterator baseIterator;
    };

    /// \brief Sets keyboard qualifier buttons mask at the end of the observed period.
    void SetCurrentQualifiersMask (QualifiersMask _mask) noexcept;

    /// \return Keyboard qualifier buttons mask at the end of the observed period.
    [[nodiscard]] QualifiersMask GetCurrentQualifiersMask () const noexcept;

    /// \return Iterator that points to the recorded events beginning.
    [[nodiscard]] EventIterator EventsBegin () noexcept;

    /// \return Iterator that points to the recorded events ending.
    [[nodiscard]] EventIterator EventsEnd () noexcept;

    /// \brief Pushes new event to the back of the input events sequence.
    void RecordEvent (const InputEvent &_event) noexcept;

    /// \brief Removes all stored input events and resets qualifier mask to zero.
    void Clear () noexcept;

private:
    QualifiersMask qualifiersMask = 0u;
    Container::Vector<InputEvent> events {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"FrameInputAccumulator"}}};
};
} // namespace Emergence::InputStorage
