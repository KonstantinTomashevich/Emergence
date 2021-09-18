#include <sstream>

#include <Export/Graph.hpp>
#include <Export/Graph/Test/Expectation.hpp>
#include <Export/Graph/Test/Export.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Export::Graph;
using namespace Emergence::Export::Graph::Test;

bool Emergence::Export::Graph::Test::ExportTestIncludeMarker () noexcept
{
    return false;
}

namespace Emergence::Export::Graph::Test::Case
{
static VisualGraph::Graph TwoConnectedNodes () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"b", {}}},
            {
                {"a", "b", {}},
            }};
}

static VisualGraph::Graph TwoConnectedNodesAndLabels () noexcept
{
    return {"graph",
            "Cool Graph",
            {},
            {{"a", "Node A"}, {"b", "Node B"}},
            {
                {"a", "b", {}},
            }};
}

static VisualGraph::Graph ChangeId (VisualGraph::Graph _graph, std::string _newId)
{
    _graph.id = std::move (_newId);
    return _graph;
}

static VisualGraph::Graph TwoSeparateSubgraphs () noexcept
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

static VisualGraph::Graph TwoInterconnectedSubgraphs () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "first_subgraph"),
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
            },
            {},
            {{"first_subgraph/a", "second_subgraph/b", {}}}};
}

static VisualGraph::Graph RelativePathsFromRoot () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoConnectedNodes (), "subgraph"),
            },
            {{"a", {}}, {"b", {}}},
            {{"a", "subgraph/a", {}}, {"b", "subgraph/b", {}}}};
}

static VisualGraph::Graph DoubleSubgraphNesting () noexcept
{
    return {"complex_graph",
            {},
            {
                ChangeId (TwoInterconnectedSubgraphs (), "first_subgraph"),
                ChangeId (RelativePathsFromRoot (), "second_subgraph"),
            },
            {{"a", {}}, {"b", {}}},
            {{"a", "second_subgraph/b", {}},
             {"b", "second_subgraph/subgraph/b", {}},
             {"first_subgraph/first_subgraph/a", "second_subgraph/b", {}}}};
}

static VisualGraph::Graph EdgeWithAbsolutePath () noexcept
{
    VisualGraph::Graph firstSubgraph = ChangeId (TwoConnectedNodes (), "first_subgraph");
    firstSubgraph.edges.emplace_back (VisualGraph::Edge {"a", "complex_graph/second_subgraph/b", {}});

    return {"complex_graph",
            {},
            {
                firstSubgraph,
                ChangeId (TwoConnectedNodes (), "second_subgraph"),
            },
            {},
            {}};
}

static VisualGraph::Graph EdgeWithColor () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"b", {}}},
            {
                {"a", "b", "#FF0000FF"},
            }};
}

static VisualGraph::Graph WithDuplicateGraphIds () noexcept
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

static VisualGraph::Graph WithIncorrectGraphIds () noexcept
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

static VisualGraph::Graph WithDuplicateNodeIds () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"b", {}}, {"a", {}}},
            {
                {"a", "b", {}},
            }};
}

static VisualGraph::Graph WithIncorrectNodeIds () noexcept
{
    return {"graph",
            {},
            {},
            {{"a", {}}, {"\"", {}}, {"/", {}}},
            {
                {"a", "/", {}},
            }};
}
} // namespace Emergence::Export::Graph::Test::Case

BEGIN_SUITE (Export)

#define POSITIVE_CASE(Name)                                                                                            \
    TEST_CASE (Name)                                                                                                   \
    {                                                                                                                  \
        std::stringstream stream;                                                                                      \
        const bool exported = Export (Case::Name (), stream);                                                          \
        CHECK (exported);                                                                                              \
        CHECK_EQUAL (stream.str (), Expectation::Name ());                                                             \
    }

POSITIVE_CASE (TwoConnectedNodes)
POSITIVE_CASE (TwoConnectedNodesAndLabels)
POSITIVE_CASE (TwoSeparateSubgraphs)
POSITIVE_CASE (TwoInterconnectedSubgraphs)
POSITIVE_CASE (RelativePathsFromRoot)
POSITIVE_CASE (DoubleSubgraphNesting)
POSITIVE_CASE (EdgeWithAbsolutePath)
POSITIVE_CASE (EdgeWithColor)

#define NEGATIVE_CASE(Name)                                                                                            \
    TEST_CASE (Name)                                                                                                   \
    {                                                                                                                  \
        std::stringstream stream;                                                                                      \
        const bool exported = Export (Case::Name (), stream);                                                          \
        CHECK (!exported);                                                                                             \
    }

NEGATIVE_CASE (WithDuplicateGraphIds)
NEGATIVE_CASE (WithIncorrectGraphIds)
NEGATIVE_CASE (WithDuplicateNodeIds)
NEGATIVE_CASE (WithIncorrectNodeIds)

END_SUITE
