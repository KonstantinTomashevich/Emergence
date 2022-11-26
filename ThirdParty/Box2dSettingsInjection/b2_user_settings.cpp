#define _CRT_SECURE_NO_WARNINGS

#ifndef B2_USER_SETTINGS
#define B2_USER_SETTINGS
#endif

#include "box2d/b2_settings.h"

static void *(*selectedAllocator) (int32_t) = nullptr;

static void (*selectedDeallocator) (void *) = nullptr;

static void (*selectedLogger) (const char *) = nullptr;

B2_API void SetBox2dAllocators (void *(*_allocator) (int32_t), void (*_deallocator) (void *)) noexcept
{
    selectedAllocator = _allocator;
    selectedDeallocator = _deallocator;
}

B2_API void *b2Alloc (int32_t _size) noexcept
{
    return selectedAllocator (_size);
}

B2_API void b2Free (void *_memory) noexcept
{
    selectedDeallocator (_memory);
}

B2_API void SetBox2dLogger (void (*_logger) (const char *))
{
    selectedLogger = _logger;
}

B2_API void b2LogVaList (const char *_format, va_list _argList) noexcept
{
    char buffer[2048u];
    int count = vsprintf (buffer, _format, _argList);
    buffer[count] = '\0';
    selectedLogger (buffer);
}
