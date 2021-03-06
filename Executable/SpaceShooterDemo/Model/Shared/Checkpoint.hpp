#pragma once

#include <Memory/UniqueString.hpp>

struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString ASSEMBLY_STARTED;
    static const Emergence::Memory::UniqueString ASSEMBLY_FINISHED;

    static const Emergence::Memory::UniqueString CAMERA_UPDATE_STARTED;
    static const Emergence::Memory::UniqueString CAMERA_UPDATE_FINISHED;

    static const Emergence::Memory::UniqueString DAMAGE_STARTED;
    static const Emergence::Memory::UniqueString DAMAGE_FINISHED;

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

    static const Emergence::Memory::UniqueString MORTALITY_STARTED;
    static const Emergence::Memory::UniqueString MORTALITY_FINISHED;

    static const Emergence::Memory::UniqueString MOVEMENT_STARTED;
    static const Emergence::Memory::UniqueString MOVEMENT_FINISHED;

    static const Emergence::Memory::UniqueString RENDER_UPDATE_STARTED;
    static const Emergence::Memory::UniqueString RENDER_UPDATE_FINISHED;

    static const Emergence::Memory::UniqueString SHOOTING_STARTED;
    static const Emergence::Memory::UniqueString SHOOTING_FINISHED;

    static const Emergence::Memory::UniqueString SPAWN_STARTED;
    static const Emergence::Memory::UniqueString SPAWN_FINISHED;
};
