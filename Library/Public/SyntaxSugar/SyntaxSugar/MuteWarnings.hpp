#pragma once

/// \def BEGIN_MUTING_WARNINGS
/// \brief Tells compiler to ignore warnings until END_MUTING_WARNINGS is found
/// \details Third party libraries do not always follow the same warning policy as Emergence project does.
///          Therefore, we are forced to silence warnings in some third party includes.

/// \def END_MUTING_WARNINGS
/// \brief Tells compile to start detecting warnings again and therefore disables BEGIN_MUTING_WARNINGS.

/// \def BEGIN_IGNORING_PADDING_WARNING
/// \brief Tells compiler to ignore wasted padding warning until END_IGNORING_PADDING_WARNING is found.
/// \details In some cases we don't care about small amount of wasted padding, but do care about the logical order
///          of fields, for example for readability improvement. This macro allows us to ignore the warning that
///          requests reordering of fields to avoid padding issues.

/// \def END_IGNORING_PADDING_WARNING
/// \brief Tells compile to start detecting wasted padding warning again after BEGIN_IGNORING_PADDING_WARNING.

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_WARNINGS _Pragma ("warning (push, 0)")
#    define END_MUTING_WARNINGS _Pragma ("warning (pop)")
#else
// clang-format off
#    define BEGIN_MUTING_WARNINGS                                                                                      \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Winconsistent-missing-override\"")                                         \
        _Pragma ("GCC diagnostic ignored \"-Wlanguage-extension-token\"")                                              \
        _Pragma ("GCC diagnostic ignored \"-Wreturn-type-c-linkage\"")                                                 \
        _Pragma ("GCC diagnostic ignored \"-Wunknown-pragmas\"")                                                       \
        _Pragma ("GCC diagnostic ignored \"-Wunused-parameter\"")
// clang-format on

#    define END_MUTING_WARNINGS _Pragma ("GCC diagnostic pop")
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_IGNORING_PADDING_WARNING _Pragma ("warning (push)") _Pragma ("warning (disable: 4324)")
#    define END_IGNORING_PADDING_WARNING _Pragma ("warning (pop)")
#else
#    define BEGIN_IGNORING_PADDING_WARNING
#    define END_IGNORING_PADDING_WARNING
#endif
