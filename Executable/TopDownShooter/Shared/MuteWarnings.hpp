#if defined(_MSC_VER) && !defined(__clang__)
#    define BEGIN_MUTING_WARNING _Pragma ("warning (push, 0)")
#    define END_MUTING_WARNING _Pragma ("warning (pop)")
#else
// clang-format off
#    define BEGIN_MUTING_WARNING                                                                                       \
        _Pragma ("GCC diagnostic push")                                                                                \
        _Pragma ("GCC diagnostic ignored \"-Wunused-parameter\"")                                                      \
        _Pragma ("GCC diagnostic ignored \"-Winconsistent-missing-override\"")
// clang-format on

#    define END_MUTING_WARNING _Pragma ("GCC diagnostic pop")
#endif
