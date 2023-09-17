#pragma once

#include <ResourceCookingApi.hpp>

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Converts all reflection-driven resource objects in yaml format to binary format.
///        Uses intermediate cache whenever possible.
ResourceCookingApi bool BinaryConversionPass (Context &_context) noexcept;
} // namespace Emergence::Resource::Cooking
