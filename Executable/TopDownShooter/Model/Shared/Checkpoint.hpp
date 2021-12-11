#pragma once

namespace Checkpoint
{
constexpr const char *TIME_UPDATED = "TimeUpdated";
constexpr const char *INPUT_DISPATCH_BEGIN = "InputDispatchBegin";
constexpr const char *INPUT_DISPATCH_END = "InputDispatchEnd";

/// After that checkpoint only lightweight tasks that collection frame statistics like duration should be executed.
/// All other tasks should be dependencies of this checkpoint.
/// TODO: Is there more elegant way to introduce frame stats collection?
constexpr const char *FRAME_STATS_COLLECTION = "FrameStatsCollection";
} // namespace Checkpoint
