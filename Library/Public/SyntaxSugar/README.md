# SyntaxSugar

SyntaxSugar contains generic utility classes and functions:

- BlockCast: allows casting `std::array <uint8_t>` into any object, that can be stored in this array. Useful for service
  implementations.
- MappingRegistration: utility macros for
  easier [StandardLayoutMapping](../../../Service/StandardLayoutMapping/README.md)-based reflection registration.
- MuteWarnings: utility macros for muting warnings in third party headers.
