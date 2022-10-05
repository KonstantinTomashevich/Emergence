#pragma once

#include <variant>

namespace Emergence::Container
{
template <typename... Types>
using Variant = std::variant<Types...>;
} // namespace Emergence::Container
