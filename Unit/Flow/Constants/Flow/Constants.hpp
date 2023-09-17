#pragma once

#include <Flow/ConstantsProfile.hpp>

namespace Emergence::Flow
{
/// \brief Flow uses bitsets to optimize resource usage overlap checks.
///        Therefore we need to specify maximum count of resources.
static constexpr std::size_t MAX_RESOURCES = Profile::MAX_RESOURCES;

/// \brief Flow uses bitsets to optimize node reachability calculations and checking.
///        Therefore we need to specify maximum count of nodes. Node is a task or checkpoint.
static constexpr std::size_t MAX_GRAPH_NODES = Profile::MAX_GRAPH_NODES;
} // namespace Emergence::Flow
