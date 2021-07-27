#pragma once

#include <array>

/// \brief Reserves space for implementation data inside class and generates appropriate constructor.
/// \warning This macro is internal, use EMERGENCE_BIND_IMPLEMENTATION_INPLACE_WITH_COPY_CONSTRUCTION
///          or EMERGENCE_BIND_IMPLEMENTATION_INPLACE_WITH_MOVE_CONSTRUCTION instead.
#define EMERGENCE_BIND_IMPLEMENTATION_INPLACE(Class, DataMaxSize, ConstData)                                           \
static constexpr std::size_t DATA_MAX_SIZE = (DataMaxSize);                                                            \
                                                                                                                       \
explicit Class (ConstData std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;                                        \
                                                                                                                       \
/*! \brief Iterator implementation-specific data. */                                                                   \
std::array <uint8_t, DATA_MAX_SIZE> data

/// \brief Reserves DataMaxSize amount of bytes for implementation data and
///        generates constructor that accepts pointer to const data array.
#define EMERGENCE_BIND_IMPLEMENTATION_INPLACE_WITH_COPY_CONSTRUCTION(Class, DataMaxSize)                               \
EMERGENCE_BIND_IMPLEMENTATION_INPLACE(Class, DataMaxSize, const)

/// \brief Reserves DataMaxSize amount of bytes for implementation data and
///        generates constructor that accepts pointer to mutable data array.
#define EMERGENCE_BIND_IMPLEMENTATION_INPLACE_WITH_MOVE_CONSTRUCTION(Class, DataMaxSize)                               \
EMERGENCE_BIND_IMPLEMENTATION_INPLACE(Class, DataMaxSize,)

/// \brief Adds implementation handle field with brief documentation comment.
#define EMERGENCE_BIND_IMPLEMENTATION_HANDLE()                                                                         \
/*! \brief Implementation handle. */                                                                                   \
void *handle

/// \brief Adds implementation handle field and private constructor that accepts handles.
#define EMERGENCE_BIND_IMPLEMENTATION_HANDLE_WITH_CONSTRUCTOR(Class)                                                   \
explicit Class (void *_handle) noexcept;                                                                               \
                                                                                                                       \
EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()