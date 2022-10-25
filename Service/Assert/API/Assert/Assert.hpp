#pragma once

#include <cstdint>

namespace Emergence
{
/// \brief Set whether failed asserts should show interactive message boxes.
void SetIsAssertInteractive (bool _interactive) noexcept;

/// \brief Called whenever assert is failed.
/// \details Should not be called directly. Public only because of EMERGENCE_ASSERT macro.
void AssertFailed (const char *_expression, const char *_file, size_t _line) noexcept;
} // namespace Emergence

#ifndef NDEBUG
#    define EMERGENCE_ASSERT_ENABLED
#endif

// TODO: Migrate all asserts to this macro.
#ifdef EMERGENCE_ASSERT_ENABLED
/// \brief Provides assertion feature.
#    define EMERGENCE_ASSERT(...)                                                                                      \
        if (!((__VA_ARGS__)))                                                                                          \
        {                                                                                                              \
            Emergence::AssertFailed (#__VA_ARGS__, __FILE__, __LINE__);                                                \
        }
#else
#    define EMERGENCE_ASSERT(Info, ...) /* No action if asserts aren't enabled. */
#endif
