#pragma once

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Packs all resources in context into package with flat structure: only full names are preserved.
bool ProduceFlatPackage (Context &_context, const std::string_view &_packageFileName) noexcept;
} // namespace Emergence::Resource::Cooking
