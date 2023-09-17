#include <API/Common/MuteWarnings.hpp>

#include <cstdint>
#include <cstdlib>

BEGIN_MUTING_WARNINGS
#define B2_USER_SETTINGS
#include <box2d/box2d.h>
END_MUTING_WARNINGS

#include <Log/Log.hpp>

namespace Emergence::Box2dAdapter
{
static Memory::Heap box2dHeap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"Physics2d::Box2d"}}};

void *Box2dProfiledAllocation (std::int32_t _size) noexcept
{
    auto *memory = static_cast<uintptr_t *> (box2dHeap.Acquire (_size + sizeof (uintptr_t), sizeof (uintptr_t)));
    *memory = _size;
    return memory + 1u;
}

void Box2dProfiledFree (void *_memory) noexcept
{
    uintptr_t *acquiredMemory = static_cast<uintptr_t *> (_memory) - 1u;
    box2dHeap.Release (acquiredMemory, *acquiredMemory + sizeof (uintptr_t));
}

void Box2dLog (const char *_information) noexcept
{
    EMERGENCE_LOG (INFO, "Physics2d::Box2d: ", _information);
}

[[maybe_unused]] static const struct Initializer
{
    Initializer ()
    {
        SetBox2dAllocators (Box2dProfiledAllocation, Box2dProfiledFree);
        SetBox2dLogger (Box2dLog);
    }
} INITIALIZER;
} // namespace Emergence::Box2dAdapter
