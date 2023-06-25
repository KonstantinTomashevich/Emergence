#pragma once

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
bool ProduceFlatPackage (Context &_context, const std::string_view &_packageFileName) noexcept;
} // namespace Emergence::Resource::Cooking
