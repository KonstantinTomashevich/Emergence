#pragma once

#include <cstdint>

#include <Container/Vector.hpp>

#include <Input/InputSingleton.hpp>

/// \brief Lists input types, supported by InputAccumulator.
enum class InputType
{
    KEYBOARD = 0u,
};

/// \brief Contains info about input event, stored in InputAccumulator.
struct InputEvent final
{
    InputType type;

    union
    {
        struct
        {
            ScanCode scan;
            KeyCode key;
            bool down;
            QualifiersMask qualifiers;
        } keyboard;
    };
};

/// \brief Accumulates input events for future processing.
/// \details Designed to be used as bridge between engine-level input provider,
///          like Urho3D or SDL2, and engine-independent code.
class InputAccumulator final
{
public:
    [[nodiscard]] const Emergence::Container::Vector<InputEvent> &GetAccumulatedEvents () const noexcept;

    void PostEvent (const InputEvent &_event) noexcept;

    void Clear () noexcept;

private:
    Emergence::Container::Vector<InputEvent> accumulatedEvents {
        Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::UniqueString {"InputAccumulator"}}};
};
