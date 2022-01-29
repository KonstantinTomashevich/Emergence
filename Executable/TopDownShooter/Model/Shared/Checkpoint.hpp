#pragma once

#include <Memory/UniqueString.hpp>

struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString TIME_UPDATED;

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

    /// After that checkpoint only lightweight tasks that collection frame statistics like duration should be executed.
    /// All other tasks should be dependencies of this checkpoint.
    /// TODO: Is there more elegant way to introduce frame stats collection?
    static const Emergence::Memory::UniqueString FRAME_STATS_COLLECTION;
};
