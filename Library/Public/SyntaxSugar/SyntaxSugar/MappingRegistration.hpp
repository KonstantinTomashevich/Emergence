#pragma once

#include <cstdint>

#include <StandardLayout/MappingBuilder.hpp>

/// \brief Helper for mapping static registration. Contains beginning of registration functor.
/// \invariant Class reflection structure name must be Class::Reflection.
/// \see EMERGENCE_MAPPING_REGISTRATION_END
#define EMERGENCE_MAPPING_REGISTRATION_BEGIN(Class)                                                                    \
    Emergence::StandardLayout::MappingBuilder builder;                                                                 \
    builder.Begin (#Class, sizeof (Class));                                                                            \
    using Type = Class;                                                                                                \
                                                                                                                       \
    return Class::Reflection                                                                                           \
    {
/// \brief Helper for mapping static registration. Contains ending of registration functor.
/// \invariant Class reflection structure name must contain `mapping` field, in which resulting mapping will be stored.
/// \see EMERGENCE_MAPPING_REGISTRATION_BEGIN
#define EMERGENCE_MAPPING_REGISTRATION_END()                                                                           \
    .mapping = builder.End (),                                                                                         \
    }                                                                                                                  \
    ;

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BIT.
/// \details Registers bit with `_name` that resides in `_baseField` byte with `_bitOffset`.
/// \invariant Class reflection structure name must contain `_name` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BIT(_name, _baseField, _bitOffset)                                                  \
    ._name = builder.RegisterBit (#_name, offsetof (Type, _baseField), _bitOffset),

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::INT,
///        FieldArchetype::UINT or FieldArchetype::FLOAT.
/// \invariant Class must contain `_field` field.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_REGULAR(_field)                                                                     \
    ._field = (builder.*Emergence::MappingRegistration::RegularFieldRegistrar<decltype (Type::_field)>::Register) (    \
        #_field, offsetof (Type, _field)),

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::STRING.
/// \invariant Class must contain `_field` field of type `std::array`.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_STRING(_field)                                                                      \
    ._field =                                                                                                          \
        builder.RegisterString (#_field, offsetof (Type, _field),                                                      \
                                sizeof (decltype (Type::_field)::value_type) *                                         \
                                    Emergence::MappingRegistration::ExtractArraySize<decltype (Type::_field)>::VALUE),

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BLOCK.
/// \invariant Class must contain `_field` field of type `std::array`.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BLOCK(_field)                                                                       \
    ._field =                                                                                                          \
        builder.RegisterBlock (#_field, offsetof (Type, _field),                                                       \
                               sizeof (decltype (Type::_field)::value_type) *                                          \
                                   Emergence::MappingRegistration::ExtractArraySize<decltype (Type::_field)>::VALUE),

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::NESTED_OBJECT.
/// \invariant Class must contain `_field` field of any type, that has static Reflect method that
///            returns reflection structure for this class.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT(_field)                                                               \
    ._field =                                                                                                          \
        builder.RegisterNestedObject (#_field, offsetof (Type, _field), decltype (Type::_field)::Reflect ().mapping),

namespace Emergence::MappingRegistration
{
/// \brief Utility structure, used to detect registration method for regular fields.
/// \see EMERGENCE_MAPPING_REGISTER_REGULAR
template <typename Type>
struct RegularFieldRegistrar;

#define ADD_REGULAR_REGISTRAR(Type, Suffix)                                                                            \
    template <>                                                                                                        \
    struct RegularFieldRegistrar<Type>                                                                                 \
    {                                                                                                                  \
        static constexpr auto Register = &StandardLayout::MappingBuilder::Register##Suffix;                            \
    }

ADD_REGULAR_REGISTRAR (int8_t, Int8);
ADD_REGULAR_REGISTRAR (int16_t, Int16);
ADD_REGULAR_REGISTRAR (int32_t, Int32);
ADD_REGULAR_REGISTRAR (int64_t, Int64);

ADD_REGULAR_REGISTRAR (uint8_t, UInt8);
ADD_REGULAR_REGISTRAR (uint16_t, UInt16);
ADD_REGULAR_REGISTRAR (uint32_t, UInt32);
ADD_REGULAR_REGISTRAR (uint64_t, UInt64);

ADD_REGULAR_REGISTRAR (float, Float);
ADD_REGULAR_REGISTRAR (double, Double);

#undef ADD_REGULAR_REGISTRAR

/// \brief Utility structure, used to detect string and block fields sizes.
/// \see EMERGENCE_MAPPING_REGISTER_STRING
/// \see EMERGENCE_MAPPING_REGISTER_BLOCK
template <typename>
struct ExtractArraySize;

template <typename Type, std::size_t Size>
struct ExtractArraySize<std::array<Type, Size>>
{
    static constexpr std::size_t VALUE = Size;
};
} // namespace Emergence::MappingRegistration
