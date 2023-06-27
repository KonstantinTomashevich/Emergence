#pragma once

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Imports all resources from initial resource provider to resource list.
bool AllResourceImportPass (Context &_context) noexcept;
} // namespace Emergence::Resource::Cooking
