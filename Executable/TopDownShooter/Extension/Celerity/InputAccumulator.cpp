#include <Celerity/InputAccumulator.hpp>

namespace Emergence::Celerity
{
const Container::Vector<InputEvent> &InputAccumulator::GetAccumulatedEvents () const noexcept
{
    return accumulatedEvents;
}

void InputAccumulator::PostEvent (const InputEvent &_event) noexcept
{
    accumulatedEvents.emplace_back (_event);
}

void InputAccumulator::Clear () noexcept
{
    accumulatedEvents.clear ();
}
} // namespace Emergence::Celerity
