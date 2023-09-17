#pragma once

#include <ResourceCookingApi.hpp>

#include <Resource/Cooking/Context.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Serializes mount list with given group name as one of the final results.
ResourceCookingApi bool ProduceMountList (Context &_context,
                                          const std::string_view &_groupName,
                                          const VirtualFileSystem::MountConfigurationList &_list) noexcept;
} // namespace Emergence::Resource::Cooking
