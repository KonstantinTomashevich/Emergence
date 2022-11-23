#pragma once

#include <cstdarg>
#include <cstdint>
#include <limits>

#define b2_lengthUnitsPerMeter 1.0f

#define b2_maxPolygonVertices 8

struct B2_API b2BodyUserData
{
    b2BodyUserData ()
    {
        objectId = std::numeric_limits<std::uint64_t>::max ();
    }

    std::uint64_t objectId;
};

struct B2_API b2FixtureUserData
{
    b2FixtureUserData ()
    {
        shapeId = std::numeric_limits<std::uint64_t>::max ();
    }

    std::uint64_t shapeId;
};

struct B2_API b2JointUserData
{
    b2JointUserData ()
    {
        jointId = std::numeric_limits<std::uint64_t>::max ();
    }

    std::uint64_t jointId;
};

B2_API void SetBox2dAllocators (void *(*_allocator) (std::int32_t), void (*_deallocator) (void *)) noexcept;

B2_API void *b2Alloc (std::int32_t _size) noexcept;

B2_API void b2Free (void *_memory) noexcept;

B2_API void b2LogVaList (const char *_format, va_list _argList) noexcept;

B2_API void SetBox2dLogger (void (*_logger) (const char *));

inline void b2Log (const char *_format, ...) noexcept
{
    va_list args;
    va_start (args, _format);
    b2LogVaList (_format, args);
    va_end (args);
}
