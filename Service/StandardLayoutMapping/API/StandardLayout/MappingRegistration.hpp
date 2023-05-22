#pragma once

#include <cstdint>

#include <Assert/Assert.hpp>

#include <Container/Optional.hpp>
#include <Container/StringBuilder.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Patch.hpp>

/// \brief Version of EMERGENCE_MAPPING_REGISTRATION_BEGIN, that allows custom class name to be passed.
#define EMERGENCE_MAPPING_REGISTRATION_BEGIN_WITH_CUSTOM_NAME(Class, ClassName)                                        \
    Emergence::StandardLayout::MappingBuilder builder;                                                                 \
    builder.Begin (Emergence::Memory::UniqueString {ClassName}, sizeof (Class), alignof (Class));                      \
    using Type [[maybe_unused]] = Class;                                                                               \
                                                                                                                       \
    if constexpr (std::is_default_constructible_v<Class> && !std::is_trivially_default_constructible_v<Class>)         \
    {                                                                                                                  \
        builder.SetConstructor (&Emergence::StandardLayout::Registration::DefaultConstructor<Class>);                  \
    }                                                                                                                  \
                                                                                                                       \
    if constexpr (std::is_move_constructible_v<Class>)                                                                 \
    {                                                                                                                  \
        builder.SetMoveConstructor (&Emergence::StandardLayout::Registration::DefaultMoveConstructor<Class>);          \
    }                                                                                                                  \
                                                                                                                       \
    if constexpr (!std::is_trivially_destructible_v<Class>)                                                            \
    {                                                                                                                  \
        builder.SetDestructor (&Emergence::StandardLayout::Registration::DefaultDestructor<Class>);                    \
    }                                                                                                                  \
                                                                                                                       \
    Class::Reflection reflectionData

/// \brief Helper for mapping static registration. Contains beginning of registration functor.
/// \invariant Class reflection structure name must be Class::Reflection.
/// \see EMERGENCE_MAPPING_REGISTRATION_END
#define EMERGENCE_MAPPING_REGISTRATION_BEGIN(Class)                                                                    \
    EMERGENCE_MAPPING_REGISTRATION_BEGIN_WITH_CUSTOM_NAME (Class, #Class)

/// \brief Helper for mapping static registration. Contains ending of registration functor.
/// \invariant Class reflection structure name must contain `mapping` field, in which resulting mapping will be stored.
/// \see EMERGENCE_MAPPING_REGISTRATION_BEGIN
#define EMERGENCE_MAPPING_REGISTRATION_END()                                                                           \
    reflectionData.mapping = builder.End ();                                                                           \
    return reflectionData

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BIT.
/// \details Registers bit with `_name` that resides in `_baseField` byte with `_bitOffset`.
/// \invariant Class reflection structure name must contain `_name` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BIT(_name, _baseField, _bitOffset)                                                  \
    reflectionData._name =                                                                                             \
        builder.RegisterBit (Emergence::Memory::UniqueString {#_name}, offsetof (Type, _baseField), _bitOffset)

/// \brief Helper for mapping static registration. Registers any field which type is
///        Emergence::StandardLayout::Registration::RegularFieldType.
/// \invariant Class reflection structure name must contain `_name` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_REGULAR(_field)                                                                     \
    reflectionData._field = Emergence::StandardLayout::Registration::RegisterRegularField<decltype (_field)> (         \
        builder, #_field, offsetof (Type, _field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::STRING.
/// \invariant Class must contain `_field` field of any std::array-based type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_STRING(_field)                                                                      \
    reflectionData._field = builder.RegisterString (Emergence::Memory::UniqueString {#_field},                         \
                                                    offsetof (Type, _field), sizeof (Type::_field))

/// \brief Helper for mapping static registration. Registers field with FieldArchetype::BLOCK.
/// \invariant Class must contain `_field` field of any std::array-based type.
/// \invariant Class reflection structure name must contain `_field` field, in which registered field id will be stored.
#define EMERGENCE_MAPPING_REGISTER_BLOCK(_field)                                                                       \
    reflectionData._field = builder.RegisterBlock (Emergence::Memory::UniqueString {#_field}, offsetof (Type, _field), \
                                                   sizeof (Type::_field))

/// \brief Helper for mapping static registration. Registers array of values where every value could be registered
///        using EMERGENCE_MAPPING_REGISTER_REGULAR.
/// \invariant Class must contain `_field` field of type `std::array`.
/// \invariant Class reflection structure name must contain `_field` field,
///            in which registered field id for each array element will be stored.
#define EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY(_field)                                                               \
    reflectionData._field = Emergence::StandardLayout::Registration::RegisterRegularArray<decltype (Type::_field)> (   \
        builder, #_field, offsetof (Type, _field), std::nullopt)

/// \brief Helper for mapping static registration. Extends EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY logic by linking
///        vector size field and pushing size-based conditionals to mapping.
#define EMERGENCE_MAPPING_REGISTER_REGULAR_VECTOR(_field, _sizeField)                                                  \
    reflectionData._field = Emergence::StandardLayout::Registration::RegisterRegularArray<decltype (Type::_field)> (   \
        builder, #_field, offsetof (Type, _field), reflectionData._sizeField)

/// \brief Helper for mapping static registration. Adds visibility condition that makes fields below visible only
///        if given field (that must be registered before) has given value.
/// \invariant `_selectorField` belongs to FieldArchetype::UINT.
#define EMERGENCE_MAPPING_UNION_VARIANT_BEGIN(_selectorField, _switchValue)                                            \
    builder.PushVisibilityCondition (reflectionData._selectorField,                                                    \
                                     Emergence::StandardLayout::ConditionalOperation::EQUAL, _switchValue)

/// \brief Helper for mapping static registration. Pops condition pushed by EMERGENCE_MAPPING_UNION_VARIANT_BEGIN.
#define EMERGENCE_MAPPING_UNION_VARIANT_END() builder.PopVisibilityCondition ()

namespace Emergence::StandardLayout::Registration
{
/// \brief Templated default constructor for objects that need it. See MappingBuilder::SetConstructor.
template <typename T>
void DefaultConstructor (void *_address)
{
    new (_address) T {};
}

/// \brief Templated default move constructor for objects that need it. See MappingBuilder::SetConstructor.
template <typename T>
void DefaultMoveConstructor (void *_address, void *_sourceAddress)
{
    new (_address) T {std::move (*static_cast<T *> (_sourceAddress))};
}

/// \brief Templated default destructor for objects that need it. See MappingBuilder::SetDestructor.
template <typename T>
void DefaultDestructor (void *_address)
{
    static_cast<T *> (_address)->~T ();
}

/// \brief Checks that type has static `Reflect` method for reflection-based logic.
template <typename T>
concept HasReflection = requires (T) {
    {
        T::Reflect ()
    };
};

template <typename>
struct IsVector
{
    static constexpr bool VALUE = false;
};

template <typename ValueType>
struct IsVector<Container::Vector<ValueType>>
{
    static constexpr bool VALUE = true;
};

/// \brief Checks that type is supported by ::RegisterRegularField function logic.
template <typename T>
concept RegularFieldType =
    std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, bool> || std::is_same_v<T, uint8_t> ||
    std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||
    std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, Memory::UniqueString> ||
    std::is_enum_v<T> || std::is_pointer_v<T> || HasReflection<T> || std::is_same_v<T, Container::Utf8String> ||
    IsVector<T>::VALUE || std::is_same_v<T, StandardLayout::Patch>;

/// \brief Registers field which registration type can be easily deduced using constant expression.
/// \details Arrays are not supported because their registration results in more than one field id.
template <RegularFieldType Type>
inline FieldId RegisterRegularField (MappingBuilder &_builder, const char *_name, std::size_t _offset)
{
    if constexpr (std::is_same_v<Type, int8_t>)
    {
        return _builder.RegisterInt8 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, int16_t>)
    {
        return _builder.RegisterInt16 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, int32_t>)
    {
        return _builder.RegisterInt32 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, int64_t>)
    {
        return _builder.RegisterInt64 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, bool> || std::is_same_v<Type, uint8_t>)
    {
        return _builder.RegisterUInt8 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, uint16_t>)
    {
        return _builder.RegisterUInt16 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, uint32_t>)
    {
        return _builder.RegisterUInt32 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, uint64_t>)
    {
        return _builder.RegisterUInt64 (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, float>)
    {
        return _builder.RegisterFloat (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, double>)
    {
        return _builder.RegisterDouble (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_same_v<Type, Memory::UniqueString>)
    {
        return _builder.RegisterUniqueString (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (std::is_enum_v<Type>)
    {
        return RegisterRegularField<std::underlying_type_t<Type>> (_builder, _name, _offset);
    }
    else if constexpr (std::is_pointer_v<Type>)
    {
        return RegisterRegularField<std::uintptr_t> (_builder, _name, _offset);
    }
    else if constexpr (HasReflection<Type>)
    {
        return _builder.RegisterNestedObject (Memory::UniqueString {_name}, _offset, Type::Reflect ().mapping);
    }
    else if constexpr (std::is_same_v<Type, Container::Utf8String>)
    {
        return _builder.RegisterUtf8String (Memory::UniqueString {_name}, _offset);
    }
    else if constexpr (IsVector<Type>::VALUE)
    {
        return _builder.RegisterVector (Memory::UniqueString {_name}, _offset, Type::value_type::Reflect ().mapping);
    }
    else if constexpr (std::is_same_v<Type, StandardLayout::Patch>)
    {
        return _builder.RegisterPatch (Memory::UniqueString {_name}, _offset);
    }
    else
    {
        // Unfortunately, we cannot use static_assert here.
        EMERGENCE_ASSERT (false);
        return 0u;
    }
}

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

/// \brief Registers array of regular fields.
/// \see ::RegisterRegularField
template <typename ArrayType>
auto RegisterRegularArray (MappingBuilder &_builder,
                           const char *_fieldName,
                           std::size_t _arrayOffset,
                           Container::Optional<FieldId> _sizeField)
{
    static_assert (RegularFieldType<typename ArrayType::value_type>);
    std::array<StandardLayout::FieldId, ExtractArraySize<ArrayType>::VALUE> result;

    for (std::uint64_t index = 0u; index < result.size (); ++index)
    {
        const std::size_t offset = _arrayOffset + sizeof (typename ArrayType::value_type) * index;
        if (_sizeField)
        {
            _builder.PushVisibilityCondition (_sizeField.value (), ConditionalOperation::GREATER, index);
        }

        result[index] = RegisterRegularField<typename ArrayType::value_type> (
            _builder, EMERGENCE_BUILD_STRING (_fieldName, "[", index, "]"), offset);

        if (_sizeField)
        {
            _builder.PopVisibilityCondition ();
        }
    }

    return result;
}
} // namespace Emergence::StandardLayout::Registration
