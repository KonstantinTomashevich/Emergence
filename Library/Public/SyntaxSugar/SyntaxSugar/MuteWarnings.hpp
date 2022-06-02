#pragma once

/// \def BEGIN_MUTING_WARNINGS
/// \brief Tells compiler to ignore warnings until END_MUTING_WARNINGS is found
/// \details Third party libraries do not always follow the same warning policy as Emergence project does.
///          Therefore, we are forced to silence warnings in some third party includes.

/// \def END_MUTING_WARNINGS
/// \brief Tells compile to start detecting warnings again and therefore disables BEGIN_MUTING_WARNINGS.

#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_WARNINGS _Pragma ("warning (push, 0)")
#    define END_MUTING_WARNINGS _Pragma ("warning (pop)")
#else
// clang-format off
#    define BEGIN_MUTING_WARNINGS                                                                                      \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Winconsistent-missing-override\"")                                         \
        _Pragma ("GCC diagnostic ignored \"-Wlanguage-extension-token\"")                                              \
        _Pragma ("GCC diagnostic ignored \"-Wunknown-pragmas\"")                                                       \
        _Pragma ("GCC diagnostic ignored \"-Wunused-parameter\"")
// clang-format on

#    define END_MUTING_WARNINGS _Pragma ("GCC diagnostic pop")
#endif
