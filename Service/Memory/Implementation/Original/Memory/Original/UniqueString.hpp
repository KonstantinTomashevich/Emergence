#pragma once

#include <string_view>
#include <type_traits>

namespace Emergence::Memory::Original
{
class UniqueString final
{
public:
    UniqueString () = default;

    explicit UniqueString (const char *_string) noexcept;

    explicit UniqueString (const std::string_view &_string) noexcept;

    const char *operator* () const noexcept;

    [[nodiscard]] std::uintptr_t Hash () const noexcept;

    bool operator== (const UniqueString &_other) const;

    bool operator!= (const UniqueString &_other) const;

private:
    const char *value = nullptr;
};

static_assert (std::is_trivially_copy_constructible_v<UniqueString>);
static_assert (std::is_trivially_move_constructible_v<UniqueString>);
static_assert (std::is_trivially_destructible_v<UniqueString>);
static_assert (std::is_trivially_copy_assignable_v<UniqueString>);
static_assert (std::is_trivially_move_assignable_v<UniqueString>);
} // namespace Emergence::Memory::Original
