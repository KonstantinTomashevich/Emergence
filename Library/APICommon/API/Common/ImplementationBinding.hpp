#pragma once

#include <array>

/// \brief Reserves DataMaxSize amount of bytes for implementation data
#define EMERGENCE_BIND_IMPLEMENTATION_INPLACE(DataMaxSize)                                                             \
    static constexpr std::size_t DATA_MAX_SIZE = (DataMaxSize);                                                        \
                                                                                                                       \
    /*! \brief Iterator implementation-specific data. */                                                               \
    std::array<uint8_t, DATA_MAX_SIZE> data

/// \brief Adds implementation handle field with brief documentation comment.
#define EMERGENCE_BIND_IMPLEMENTATION_HANDLE()                                                                         \
    /*! \brief Implementation handle. */                                                                               \
    void *handle
