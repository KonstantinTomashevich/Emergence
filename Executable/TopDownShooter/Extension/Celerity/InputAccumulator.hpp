#pragma once

#include <cstdint>

#include <Container/Vector.hpp>

namespace Emergence::Celerity
{
using KeyCode = std::int32_t;

using QualifiersMask = std::uint32_t;

enum class InputType
{
    KEYBOARD = 0u,
};

struct InputEvent final
{
    InputType type;

    union
    {
        struct
        {
            KeyCode key;
            bool down;
            QualifiersMask qualifiers;
        } keyboard;
    };
};

/// \details Designed to be used as bridge between engine-level input provider,
///          like Urho3D or SDL2, and engine-independent code.
class InputAccumulator final
{
public:
    [[nodiscard]] const Container::Vector<InputEvent> &GetAccumulatedEvents () const noexcept;

    void PostEvent (const InputEvent &_event) noexcept;

    void Clear () noexcept;

private:
    Container::Vector<InputEvent> accumulatedEvents {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"InputAccumulator"}}};
};
} // namespace Emergence::Celerity
