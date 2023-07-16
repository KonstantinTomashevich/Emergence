# SyntaxSugar<sup>Concrete</sup>

SyntaxSugar contains generic utility classes and functions:

- AtomicFlagGuard: RAII guard for `std::atomic_flag`-based spin locking.
- BlockCast: allows casting `std::array <std::uint8_t>` into any object, that can be stored in this array. Useful for 
  abstract unit implementations. Also, reverse cast aka `array_cast` is provided.
- MuteWarnings: utility macros for muting warnings in third party headers.
- SelectType: Templates for type selection.
- Time: time-related utility functions.
- Union: Templates and macros for working with unions.
