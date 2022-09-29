#pragma once

#include <variant>

namespace Emergence::Container
{
// TODO: Port all std::variant usage to Container::Variant.
template <typename... Types>
using Variant = std::variant<Types...>;
} // namespace Emergence::Container
