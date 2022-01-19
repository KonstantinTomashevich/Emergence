# SyntaxSugar

SyntaxSugar contains generic utility classes and functions:

- AtomicFlagGuard: RAII guard for `std::atomic_flag`-based spin locking.
- BlockCast: allows casting `std::array <uint8_t>` into any object, that can be stored in this array. Useful for service
  implementations.
- MuteWarnings: utility macros for muting warnings in third party headers.
- Time: time-related utility functions.
