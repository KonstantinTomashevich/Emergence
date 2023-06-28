#pragma once

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Organizes all resources from the context in flat structure though weak file links in virtual directory
///        (only full names are preserved) and generates resource provider index using this virtual directory as source.
///        Generated index is registered as resource too.
bool AllResourceFlatIndexPass (Context &_context) noexcept;
} // namespace Emergence::Resource::Cooking
