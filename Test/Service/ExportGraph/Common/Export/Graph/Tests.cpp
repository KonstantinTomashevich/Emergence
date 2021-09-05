#include <Export/Graph/Tests.hpp>
#include <utility>

namespace Emergence::Export::Graph::Test
{
VisualGraph::Graph TwoConnectedNodes () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"b", {}}},
            {
                {"a", "b"},
            }};
}

VisualGraph::Graph TwoConnectedNodesAndLabels () noexcept
{
    return {"graph",
            "Cool Graph",
            {},
            {{"a", "Node A"}, {"b", "Node B"}},
            {
                {"a", "b"},
            }};
}

VisualGraph::Graph ChangeId (VisualGraph::Graph _graph, std::string _newId)
{
    _graph.id = std::move (_newId);
    return _graph;
}

VisualGraph::Graph TwoSeparateSubgraphs () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "first_subgraph"),
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
            },
            {},
            {}};
}

VisualGraph::Graph TwoInterconnectedSubgraphs () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "first_subgraph"),
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
            },
            {},
            {{"first_subgraph/a", "second_subgraph/b"}}};
}

VisualGraph::Graph RelativePathsFromRoot () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "subgraph"),
            },
            {{"a", {}}, {"b", {}}},
            {{"a", "subgraph/a"}, {"b", "subgraph/b"}}};
}

VisualGraph::Graph DoubleSubgraphNesting () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoInterconnectedSubgraphs (), "first_subgraph"),
                ChangeId (RelativePathsFromRoot (), "second_subgraph"),
            },
            {{"a", {}}, {"b", {}}},
            {{"a", "second_subgraph/b"},
             {"b", "second_subgraph/subgraph/b"},
             {"first_subgraph/first_subgraph/a", "second_subgraph/b"}}};
}

VisualGraph::Graph EdgeWithAbsolutePath () noexcept
{
    VisualGraph::Graph firstSubgraph = ChangeId (TwoConnectedNodes (), "first_subgraph");
    firstSubgraph.edges.emplace_back (VisualGraph::Edge {"a", "complex_graph/second_subgraph/b"});

    return {"complex_graph",
            {},
            {
                firstSubgraph,
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
            },
            {},
            {}};
}

VisualGraph::Graph WithDuplicateGraphIds () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "first_subgraph"),
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
                ChangeId (TwoConnectedNodes (), "first_subgraph"),
            },
            {},
            {}};
}

VisualGraph::Graph WithIncorrectGraphIds () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "/"),
                ChangeId (TwoConnectedNodes (), "\""),
                ChangeId (TwoConnectedNodes (), "subgraph"),
            },
            {},
            {}};
}

VisualGraph::Graph WithDuplicateNodeIds () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"b", {}}, {"a", {}}},
            {
                {"a", "b"},
            }};
}

VisualGraph::Graph WithIncorrectNodeIds () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"\"", {}}, {"/", {}}},
            {
                {"a", "/"},
            }};
}
} // namespace Emergence::Export::Graph::Test
