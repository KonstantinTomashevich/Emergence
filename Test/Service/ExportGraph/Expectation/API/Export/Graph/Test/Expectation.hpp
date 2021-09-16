#pragma once

#include <string>

namespace Emergence::Export::Graph::Test::Expectation
{
std::string TwoConnectedNodes () noexcept;

std::string TwoConnectedNodesAndLabels () noexcept;

std::string TwoSeparateSubgraphs () noexcept;

std::string TwoInterconnectedSubgraphs () noexcept;

std::string RelativePathsFromRoot () noexcept;

std::string DoubleSubgraphNesting () noexcept;

std::string EdgeWithAbsolutePath () noexcept;

std::string EdgeWithColor () noexcept;
} // namespace Emergence::Export::Graph::Test::Expectation
