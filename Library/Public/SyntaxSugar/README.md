# SyntaxSugar

SyntaxSugar contains generic utility classes and functions:

- BlockCast: allows casting `std::array <uint8_t>` into any object, that can be stored in this array. Useful for service
  implementations.
- InplaceVector: dynamic stack-allocated array with fixed max size, built on top of std::array.
- MappingRegistration: utility macros for
  easier [StandardLayoutMapping](../../../Service/StandardLayoutMapping/README.md)-based reflection registration.
