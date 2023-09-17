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
void *Box2dEffectiveAllocation (std::int32_t _size) noexcept
{
    return malloc (_size);
}

void Box2dEffectiveFree (void *_memory) noexcept
{
    free (_memory);
}

void Box2dLog (const char *_information) noexcept
{
    EMERGENCE_LOG (INFO, "Physics2d::Box2d: ", _information);
}

[[maybe_unused]] static const struct Initializer
{
    Initializer ()
    {
        SetBox2dAllocators (Box2dEffectiveAllocation, Box2dEffectiveFree);
        SetBox2dLogger (Box2dLog);
    }
} INITIALIZER;
} // namespace Emergence::Box2dAdapter
