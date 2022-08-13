#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Input/InputAccumulator.hpp>

namespace Input
{
/// \details Planned input routine:
//           - External event registration allowed (UI, replays).
//           - DISPATCH_STARTED.
//           - Listeners are cleared.
//           - Input is captured.
//           - Listeners are populated by native events.
//           - LISTENERS_PUSH_ALLOWED.
//           - External logic can push events directly to listeners (AI).
//           - LISTENERS_READ_ALLOWED.
//           - Game logic can finally process input events.
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString DISPATCH_STARTED;
    static const Emergence::Memory::UniqueString LISTENERS_PUSH_ALLOWED;
    static const Emergence::Memory::UniqueString LISTENERS_READ_ALLOWED;
};

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (InputAccumulator *_inputAccumulator,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Input
