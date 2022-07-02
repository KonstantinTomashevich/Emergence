#pragma once

#include <cassert>
#include <cstdint>

#include <Container/InplaceVector.hpp>
#include <Container/StringBuilder.hpp>

#include <StandardLayout/MappingBuilder.hpp>

/// \brief Helper for mapping static registration. Contains beginning of registration functor.
/// \invariant Class reflection structure name must be Class::Reflection.
/// \see EMERGENCE_MAPPING_REGISTRATION_END
#define EMERGENCE_MAPPING_REGISTRATION_BEGIN(Class)                                                                    \
    Emergence::StandardLayout::MappingBuilder builder;                                                                 \
    builder.Begin (Emergence::Memory::UniqueString {#Class}, sizeof (Class), alignof (Class));                         \
    using Type [[maybe_unused]] = Class;                                                                               \
                                                                                                                       \
    if constexpr (std::is_default_constructible_v<Class> && !std::is_trivially_default_constructible_v<Class>)         \
    {                                                                                                                  \
        builder.SetConstructor (&Emergence::StandardLayout::Registration::DefaultConstructor<Class>);                  \
    }                                                                                                                  \
                                                                                                                       \
    if constexpr (!std::is_trivially_destructible_v<Class>)                                                            \
    {                                                                                                                  \
        builder.SetDestructor (&Emergence::StandardLayout::Registration::DefaultDestructor<Class>);                    \
    }                                                                                                                  \
                                                                                                                       \
    Class::Reflection reflection

/// \brief Helper for mapping static registration. Contains ending of registration functor.
/// \invariant Class reflection structure name must contain `mapping` field, in which resulting mapping will be stored.
/// \see EMERGENCE_MAPPING_REGISTRATION_BEGIN
#define EMERGENCE_MAPPING_REGISTRATION_END()                                                                           \
    reflection.mapping = builder.End ();                                                                               \
    return reflection

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BIT.
/// \details Registers bit with `_name` that resides in `_baseField` byte with `_bitOffset`.
/// \invariant Class reflection structure name must contain `_name` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BIT(_name, _baseField, _bitOffset)                                                  \
    reflection._name =                                                                                                 \
        builder.RegisterBit (Emergence::Memory::UniqueString {#_name}, offsetof (Type, _baseField), _bitOffset)

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::INT,
///        FieldArchetype::UINT or FieldArchetype::FLOAT.
/// \invariant Class must contain `_field` field.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_REGULAR(_field)                                                                     \
    reflection._field =                                                                                                \
        (builder.*Emergence::StandardLayout::Registration::RegularFieldRegistrar<decltype (Type::_field)>::Register) ( \
            Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field))

/// \brief Helper for mapping static registration. Registers enum field as field with FieldArchetype::INT,
///        FieldArchetype::UINT or FieldArchetype::FLOAT.
/// \invariant Class must contain `_field` field.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR(_field)                                                             \
    reflection._field = (builder.*Emergence::StandardLayout::Registration::RegularFieldRegistrar<                      \
                                      std::underlying_type_t<decltype (Type::_field)>>::Register) (                    \
        Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field))

/// \brief Helper for mapping static registration. Registers pointer field as field with FieldArchetype::UINT.
/// \invariant Class must contain `_field` field.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR(_field)                                                          \
    reflection._field =                                                                                                \
        (builder.*Emergence::StandardLayout::Registration::RegularFieldRegistrar<uintptr_t>::Register) (               \
            Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::STRING.
/// \invariant Class must contain `_field` field of any std::array-based type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_STRING(_field)                                                                      \
    reflection._field = builder.RegisterString (Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field),    \
                                                sizeof (Type::_field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BLOCK.
/// \invariant Class must contain `_field` field of any std::array-based type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BLOCK(_field)                                                                       \
    reflection._field = builder.RegisterBlock (Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field),     \
                                               sizeof (Type::_field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::UNIQUE_STRING.
/// \invariant Class must contain `_field` field of Emergence::Memory::UniqueString type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING(_field)                                                               \
    reflection._field =                                                                                                \
        builder.RegisterUniqueString (Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::NESTED_OBJECT.
/// \invariant Class must contain `_field` field of any type, that has static Reflect method that
///            returns reflection structure for this class.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT(_field)                                                               \
    reflection._field =                                                                                                \
        builder.RegisterNestedObject (Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field),              \
                                      decltype (Type::_field)::Reflect ().mapping)

/// \brief Helper for mapping static registration. Registers array fields with FieldArchetype::INT,
///        FieldArchetype::UINT or FieldArchetype::FLOAT elements.
/// \invariant Class must contain `_field` field of type `std::array`.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id for each array
///            element will be stored, and `_field`Block field, in which full array data block as field will be stored.
#define EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY(_field)                                                               \
    reflection._field##Block = builder.RegisterBlock (Emergence::Memory::UniqueString {#_field},                       \
                                                      offsetof (Type, _field), sizeof (Type::_field)),                 \
    reflection._field = Emergence::StandardLayout::Registration::RegisterArray<decltype (Type::_field)> (              \
        #_field,                                                                                                       \
        [&builder] (std::size_t _offset, Emergence::Memory::UniqueString _itemName)                                    \
        {                                                                                                              \
            using ItemType = decltype (Type::_field)::value_type;                                                      \
            using Registrar = Emergence::StandardLayout::Registration::RegularFieldRegistrar<ItemType>;                \
            return (builder.*Registrar::Register) (_itemName, offsetof (Type, _field) + _offset);                      \
        })

/// \brief Helper for mapping static registration. Registers array fields with FieldArchetype::NESTED_OBJECT elements.
/// \invariant Class must contain `_field` field of type `std::array`. Element type should have static Reflect method
///            that returns reflection structure for that type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id for each array
///            element will be stored, and `_field`Block field, in which full array data block as field will be stored.
#define EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY(_field)                                                         \
    reflection._field##Block = builder.RegisterBlock (Emergence::Memory::UniqueString {#_field},                       \
                                                      offsetof (Type, _field), sizeof (Type::_field)),                 \
    reflection._field = Emergence::StandardLayout::Registration::RegisterArray<decltype (Type::_field)> (              \
        #_field,                                                                                                       \
        [&builder] (std::size_t _offset, Emergence::Memory::UniqueString _itemName)                                    \
        {                                                                                                              \
            using ItemType = decltype (Type::_field)::value_type;                                                      \
            return builder.RegisterNestedObject (_itemName, offsetof (Type, _field) + _offset,                         \
                                                 ItemType::Reflect ().mapping);                                        \
        })

/// \brief Helper for mapping static registration. Registers array fields with FieldArchetype::UNIQUE_STRING elements.
/// \invariant Class must contain `_field` field of type `std::array`.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id for each array
///            element will be stored, and `_field`Block field, in which full array data block as field will be stored.
#define EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING_ARRAY(_field)                                                         \
    reflection._field##Block = builder.RegisterBlock (Emergence::Memory::UniqueString {#_field},                       \
                                                      offsetof (Type, _field), sizeof (Type::_field)),                 \
    reflection._field = Emergence::StandardLayout::Registration::RegisterArray<decltype (Type::_field)> (              \
        #_field,                                                                                                       \
        [&builder] (std::size_t _offset, Emergence::Memory::UniqueString _itemName)                                    \
        {                                                                                                              \
            return builder.RegisterUniqueString (_itemName, offsetof (Type, _field) + _offset);                        \
        })

namespace Emergence::StandardLayout::Registration
{
/// \brief Templated default constructor for objects that need it. See MappingBuilder::SetConstructor.
template <typename T>
void DefaultConstructor (void *_address)
{
    new (_address) T {};
}

/// \brief Templated default destructor for objects that need it. See MappingBuilder::SetDestructor.
template <typename T>
void DefaultDestructor (void *_address)
{
    static_cast<T *> (_address)->~T ();
}

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

/// \details Boolean fields are stored as one byte, therefore we register them as unsigned bytes.
ADD_REGULAR_REGISTRAR (bool, UInt8);

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

template <typename Type, std::size_t Capacity>
struct ExtractArraySize<Container::InplaceVector<Type, Capacity>>
{
    static constexpr std::size_t VALUE = Capacity;
};

template <typename>
struct ExtractArrayFirstElementOffset;

template <typename Type, std::size_t Size>
struct ExtractArrayFirstElementOffset<std::array<Type, Size>>
{
    static constexpr std::size_t VALUE = 0u;
};

template <typename Type, std::size_t Capacity>
struct ExtractArrayFirstElementOffset<Container::InplaceVector<Type, Capacity>>
{
    static constexpr std::size_t VALUE = Container::InplaceVector<Type, Capacity>::FIRST_ITEM_OFFSET;
};

template <typename ArrayType, typename ElementRegistrar>
auto RegisterArray (const char *_fieldName, const ElementRegistrar &_elementRegistrar)
{
    std::array<StandardLayout::FieldId, ExtractArraySize<ArrayType>::VALUE> result;
    const std::size_t firstElementOffset = ExtractArrayFirstElementOffset<ArrayType>::VALUE;

    for (std::size_t index = 0u; index < result.size (); ++index)
    {
        result[index] =
            _elementRegistrar (firstElementOffset + sizeof (typename ArrayType::value_type) * index,
                               Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (_fieldName, "[", index, "]")});
    }

    return result;
}
} // namespace Emergence::StandardLayout::Registration
