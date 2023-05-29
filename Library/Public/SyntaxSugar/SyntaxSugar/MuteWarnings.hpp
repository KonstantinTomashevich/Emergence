#pragma once

/// \def BEGIN_MUTING_WARNINGS
/// \brief Tells compiler to ignore warnings until END_MUTING_WARNINGS is found
/// \details Third party libraries do not always follow the same warning policy as Emergence project does.
///          Therefore, we are forced to silence warnings in some third party includes.

/// \def END_MUTING_WARNINGS
/// \brief Tells compile to start detecting warnings again and therefore disables BEGIN_MUTING_WARNINGS.

/// \def BEGIN_MUTING_UNKNOWN_ATTRIBUTE_WARNINGS
/// \brief Tells compiler to ignore unknown attributes without raising an error.
/// \details Needed to silence old GCC versions that do not support [[maybe_unused]] on fields.

/// \def BEGIN_MUTING_NO_RETURN_WARNINGS
/// \brief Tells compiler to ignore no-return method with return warning.
/// \details Needed because we cannot support no-return behaviour as we do not support exceptions.

/// \def BEGIN_MUTING_STRING_ALIASING_WARNINGS
/// \brief Tells compiler to ignore code that doesn't follow strict aliasing.
/// \details Needed because we don't follow strict aliasing in some very rare cases due to reinterpret casts.

/// \def BEGIN_MUTING_PADDING_WARNING
/// \brief Tells compiler to ignore wasted padding warning until END_IGNORING_PADDING_WARNING is found.
/// \details In some cases we don't care about small amount of wasted padding, but do care about the logical order
///          of fields, for example for readability improvement. This macro allows us to ignore the warning that
///          requests reordering of fields to avoid padding issues.

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_WARNINGS _Pragma ("warning (push, 0)")
#    define END_MUTING_WARNINGS _Pragma ("warning (pop)")
#else
// clang-format off
#    define BEGIN_MUTING_WARNINGS                                                                                      \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wpragmas\"")                                                               \
        _Pragma ("GCC diagnostic ignored \"-Wdeprecated-copy\"")                                                       \
        _Pragma ("GCC diagnostic ignored \"-Wdeprecated-copy-with-user-provided-copy\"")                               \
        _Pragma ("GCC diagnostic ignored \"-Winconsistent-missing-override\"")                                         \
        _Pragma ("GCC diagnostic ignored \"-Wlanguage-extension-token\"")                                              \
        _Pragma ("GCC diagnostic ignored \"-Wreturn-type-c-linkage\"")                                                 \
        _Pragma ("GCC diagnostic ignored \"-Wunknown-pragmas\"")                                                       \
        _Pragma ("GCC diagnostic ignored \"-Wunused-parameter\"")
// clang-format on

#    define END_MUTING_WARNINGS _Pragma ("GCC diagnostic pop")
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_UNKNOWN_ATTRIBUTE_WARNINGS BEGIN_MUTING_WARNINGS
#else
// clang-format off
#    define BEGIN_MUTING_UNKNOWN_ATTRIBUTE_WARNINGS                                                                    \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wpragmas\"")                                                               \
        _Pragma ("GCC diagnostic ignored \"-Wattributes\"")
// clang-format on
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_OLD_DESTRUCTOR_NAME BEGIN_MUTING_WARNINGS
#else
// clang-format off
#    define BEGIN_MUTING_OLD_DESTRUCTOR_NAME                                                                           \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wpragmas\"")                                                               \
        _Pragma ("GCC diagnostic ignored \"-Wdtor-name\"")
// clang-format on
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_NO_RETURN_WARNINGS _Pragma ("warning (push, 0)")
#else
// clang-format off
#    define BEGIN_MUTING_NO_RETURN_WARNINGS                                                                            \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wpragmas\"")                                                               \
        _Pragma ("GCC diagnostic ignored \"-Winvalid-noreturn\"")
// clang-format on
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_STRING_ALIASING_WARNINGS BEGIN_MUTING_WARNINGS
#else
// clang-format off
#    define BEGIN_MUTING_STRING_ALIASING_WARNINGS                                                                      \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wpragmas\"")                                                               \
        _Pragma ("GCC diagnostic ignored \"-Wstrict-aliasing\"")
// clang-format on
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_PADDING_WARNING _Pragma ("warning (push)") _Pragma ("warning (disable: 4324)")
#else
#    define BEGIN_MUTING_PADDING_WARNING _Pragma ("GCC diagnostic push")
#endif
