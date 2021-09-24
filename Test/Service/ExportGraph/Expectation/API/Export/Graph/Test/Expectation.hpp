#pragma once

#include <string>

namespace Emergence::Export::Graph::Test::Expectation
{
std::string TwoConnectedNodes ();

std::string TwoConnectedNodesAndLabels ();

std::string TwoSeparateSubgraphs ();

std::string TwoInterconnectedSubgraphs ();

std::string RelativePathsFromRoot ();

std::string DoubleSubgraphNesting ();

std::string EdgeWithAbsolutePath ();

std::string EdgeWithColor ();
} // namespace Emergence::Export::Graph::Test::Expectation
