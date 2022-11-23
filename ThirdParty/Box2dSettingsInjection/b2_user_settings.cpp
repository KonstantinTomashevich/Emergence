#define _CRT_SECURE_NO_WARNINGS

#include "box2d/b2_settings.h"

static void *(*selectedAllocator) (std::int32_t) = nullptr;
static void (*selectedDeallocator) (void *) = nullptr;
void (*selectedLogger) (const char *) = nullptr;

B2_API void SetBox2dAllocators (void *(*_allocator) (std::int32_t), void (*_deallocator) (void *)) noexcept
{
    selectedAllocator = _allocator;
    selectedDeallocator = _deallocator;
}

B2_API void *b2Alloc (std::int32_t _size) noexcept
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
