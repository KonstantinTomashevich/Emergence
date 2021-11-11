#pragma once

#include <type_traits>

namespace Emergence::String
{
class ConstReference final
{
public:
    explicit ConstReference (const std::string_view &_value) noexcept;

    [[nodiscard]] const char *Value () const noexcept;

private:
    const char *value = nullptr;
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
