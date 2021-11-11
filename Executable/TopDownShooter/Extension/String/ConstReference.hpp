#pragma once

#include <type_traits>
#include <string_view>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::String
{
class ConstReference final
{
public:
    ConstReference () = default;

    ConstReference (const char *_value) noexcept;

    ConstReference (const std::string_view &_value) noexcept;

    [[nodiscard]] const char *Value () const noexcept;

private:
    const char *value = nullptr;

public:
    struct Reflection final
    {
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::String

namespace std
{
template <>
struct hash<Emergence::String::ConstReference>
{
    std::size_t operator() (const Emergence::String::ConstReference &_reference) const noexcept
    {
        return reinterpret_cast<std::size_t> (_reference.Value ());
    }
};
} // namespace std
