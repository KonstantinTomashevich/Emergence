#pragma once

#include <Container/String.hpp>

namespace Emergence::Export::Graph::Test::Expectation
{
Container::String TwoConnectedNodes ();

Container::String TwoConnectedNodesAndLabels ();

Container::String TwoSeparateSubgraphs ();

Container::String TwoInterconnectedSubgraphs ();

Container::String RelativePathsFromRoot ();

Container::String DoubleSubgraphNesting ();

Container::String EdgeWithAbsolutePath ();

Container::String EdgeWithColor ();
} // namespace Emergence::Export::Graph::Test::Expectation
