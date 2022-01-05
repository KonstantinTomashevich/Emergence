#pragma once

#include <Memory/UniqueString.hpp>

struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString TIME_UPDATED;
    static const Emergence::Memory::UniqueString INPUT_DISPATCH_BEGIN;
    static const Emergence::Memory::UniqueString INPUT_DISPATCH_END;

    /// After that checkpoint only lightweight tasks that collection frame statistics like duration should be executed.
    /// All other tasks should be dependencies of this checkpoint.
    /// TODO: Is there more elegant way to introduce frame stats collection?
    static const Emergence::Memory::UniqueString FRAME_STATS_COLLECTION;
};
