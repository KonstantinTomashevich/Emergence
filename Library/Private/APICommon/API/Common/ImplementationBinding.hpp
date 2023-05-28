#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <array>

/// \brief Reserves DataMaxSize amount of bytes for implementation data
#define EMERGENCE_BIND_IMPLEMENTATION_INPLACE(DataMaxSize)                                                             \
    static constexpr std::size_t DATA_MAX_SIZE = (DataMaxSize);                                                        \
                                                                                                                       \
    union                                                                                                              \
    {                                                                                                                  \
        /*! \brief Iterator implementation-specific data. */                                                           \
        std::array<std::uint8_t, DATA_MAX_SIZE> data;                                                                  \
                                                                                                                       \
        /*!                                                                                                            \
         * \details Binding implementation through ::data results in 1-byte alignment,                                 \
         *          which is not suitable for most use cases. This pointer-size integer                                \
         *          is added to solve this problem by enforcing default arch-byte alignment.                           \
         */                                                                                                            \
        BEGIN_MUTING_UNKNOWN_ATTRIBUTE_WARNINGS                                                                        \
        [[maybe_unused]] std::uintptr_t alignmentFixer;                                                                \
        END_MUTING_WARNINGS                                                                                            \
    }

/// \brief Adds implementation handle field with brief documentation comment.
#define EMERGENCE_BIND_IMPLEMENTATION_HANDLE()                                                                         \
    /*! \brief Implementation handle. */                                                                               \
    void *handle /* NOLINT(bugprone-macro-parentheses): Fields can not be enclosed. */
