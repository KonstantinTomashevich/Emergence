#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include <Container/String.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Container
{
/// \brief Provides API for composite string construction without excessive allocations.
/// \details Buffered concatenation allows to optimize memory usage and improve performance of composite string
///          construction. For example, to construct `X` as `X = A + B + C + D` we need to allocate two additional
///          strings `Y = A + B` and `Z = Y + C`. Buffered concatenation allows to optimize this behaviour through
///          usage of one shared buffer.
class StringBuilder final
{
public:
    /// \details We use fixed-size buffer implementation, because it can be allocated directly on stack.
    static constexpr std::size_t BUFFER_SIZE = 512u;

    /// \brief Combines pointer to field with reflection data in order to make it printable.
    struct FieldPointer
    {
        const void *pointer;
        StandardLayout::Field reflection;
    };

    /// \brief Combines pointer to object with reflection data in order to make it printable.
    struct ObjectPointer
    {
        const void *pointer;
        StandardLayout::Mapping reflection;
    };

    StringBuilder () noexcept;

    StringBuilder (const StringBuilder &_other) noexcept = default;

    StringBuilder (StringBuilder &&_other) noexcept = default;

    ~StringBuilder () noexcept = default;

    /// \brief Moves cursor back to the beginning.
    StringBuilder &Reset () noexcept;

    /// \return Null-terminated string, currently stored in the buffer.
    [[nodiscard]] const char *Get () const noexcept;

    /// \brief Append given null terminated string to the buffer.
    StringBuilder &Append (const char *_value) noexcept;

    /// \brief Append given unique string to the buffer.
    StringBuilder &Append (Memory::UniqueString _value) noexcept;

    /// \brief Append given string to the buffer.
    StringBuilder &Append (const String &_value) noexcept;

    /// \brief Append given string view to the buffer.
    StringBuilder &Append (const std::string_view &_value) noexcept;

    /// \brief Append given boolean value to the buffer.
    StringBuilder &Append (bool _value) noexcept;

    /// \brief Append given symbol to the buffer.
    StringBuilder &Append (char _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (short _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (int _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (long _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (long long _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (unsigned short _value) noexcept;

    StringBuilder &Append (unsigned _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (unsigned long _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (unsigned long long _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (float _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (double _value) noexcept;

    /// \brief Append given number to the buffer.
    StringBuilder &Append (long double _value) noexcept;

    /// \brief Append value of pointed field using given reflection data.
    StringBuilder &Append (const FieldPointer &_reflectedField) noexcept;

    /// \brief Append value of pointed object using given reflection data.
    StringBuilder &Append (const ObjectPointer &_reflectedObject) noexcept;

    /// \brief Helper, that calls Append for each given argument.
    template <typename... Args>
    inline StringBuilder &Append (Args... _args) noexcept;

    /// \return How many symbols can be appended to the buffer?
    [[nodiscard]] std::size_t SpaceLeft () const noexcept;

    StringBuilder &operator= (const StringBuilder &_other) noexcept = default;

    StringBuilder &operator= (StringBuilder &&_other) noexcept = default;

private:
    std::array<char, BUFFER_SIZE> buffer;

    std::size_t count = 0u;
};

template <typename... Args>
inline StringBuilder &StringBuilder::Append (Args... _args) noexcept
{
    static_assert (sizeof...(_args) > 1u, "Detected infinite recursion because of unsupported argument type.");
    return (Append (_args), ...);
}
} // namespace Emergence::Container

/// \brief Concatenates given arguments using StringBuilder and returns
///        its instance, that can be used to continue string building.
#define EMERGENCE_BEGIN_BUILDING_STRING(...) Emergence::Container::StringBuilder {}.Append (__VA_ARGS__)

/// \brief Concatenates string from given arguments using StringBuilder and returns pointer to result on stack.
#define EMERGENCE_BUILD_STRING(...) EMERGENCE_BEGIN_BUILDING_STRING (__VA_ARGS__).Get ()
