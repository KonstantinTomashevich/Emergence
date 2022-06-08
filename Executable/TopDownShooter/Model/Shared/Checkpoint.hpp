#pragma once

#include <Memory/UniqueString.hpp>

struct Checkpoint final
{
    Checkpoint () = delete;

    // NOTE: Expected input routine:
    //       - External event registration allowed (UI, replays).
    //       - INPUT_DISPATCH_STARTED.
    //       - Listeners are cleared.
    //       - Input is captured.
    //       - Listeners are populated by native events.
    //       - INPUT_LISTENERS_PUSH_ALLOWED.
    //       - External logic can push events directly to listeners (AI).
    //       - INPUT_LISTENERS_READ_ALLOWED.
    //       - Game logic can finally process input events.

    static const Emergence::Memory::UniqueString INPUT_DISPATCH_STARTED;
    static const Emergence::Memory::UniqueString INPUT_LISTENERS_PUSH_ALLOWED;
    static const Emergence::Memory::UniqueString INPUT_LISTENERS_READ_ALLOWED;

    static const Emergence::Memory::UniqueString RENDER_UPDATE_STARTED;
    static const Emergence::Memory::UniqueString RENDER_UPDATE_FINISHED;
};
