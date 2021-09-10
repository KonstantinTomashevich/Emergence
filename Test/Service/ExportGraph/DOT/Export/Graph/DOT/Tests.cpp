#include <sstream>

#include <Export/Graph.hpp>

#include <Export/Graph/Tests.hpp>

#include <Testing/SetupMain.hpp>
#include <Testing/Testing.hpp>

using namespace Emergence::Export;
using namespace Emergence::Export::Graph::Test;

BEGIN_SUITE (ExportGraph)

TEST_CASE (TwoConnectedNodes)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoConnectedNodes (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"graph\" {\n"
        "    label=\"graph\";\n"
        "    \"graph/a\" [label=\"a\" ];\n"
        "    \"graph/b\" [label=\"b\" ];\n"
        "    \"graph/a\" -> \"graph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoConnectedNodesAndLabels)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoConnectedNodesAndLabels (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"graph\" {\n"
        "    label=\"Cool Graph\";\n"
        "    \"graph/a\" [label=\"Node A\" ];\n"
        "    \"graph/b\" [label=\"Node B\" ];\n"
        "    \"graph/a\" -> \"graph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoSeparateSubgraphs)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoSeparateSubgraphs (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    label=\"complex_graph\";\n"
        "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
        "        label=\"first_subgraph\";\n"
        "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
        "        label=\"second_subgraph\";\n"
        "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoInterconnectedSubgraphs)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoInterconnectedSubgraphs (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    label=\"complex_graph\";\n"
        "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
        "        label=\"first_subgraph\";\n"
        "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
        "        label=\"second_subgraph\";\n"
        "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (RelativePathsFromRoot)
{
    std::stringstream stream;
    const bool exported = Graph::Export (RelativePathsFromRoot (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    label=\"complex_graph\";\n"
        "    subgraph \"cluster_complex_graph/subgraph\" {\n"
        "        label=\"subgraph\";\n"
        "        \"complex_graph/subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    \"complex_graph/a\" [label=\"a\" ];\n"
        "    \"complex_graph/b\" [label=\"b\" ];\n"
        "    \"complex_graph/subgraph/a\" -> \"complex_graph/subgraph/b\" [];\n"
        "    \"complex_graph/a\" -> \"complex_graph/subgraph/a\" [];\n"
        "    \"complex_graph/b\" -> \"complex_graph/subgraph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (DoubleSubgraphNesting)
{
    std::stringstream stream;
    const bool exported = Graph::Export (DoubleSubgraphNesting (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    label=\"complex_graph\";\n"
        "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
        "        label=\"first_subgraph\";\n"
        "        subgraph \"cluster_complex_graph/first_subgraph/first_subgraph\" {\n"
        "            label=\"first_subgraph\";\n"
        "            \"complex_graph/first_subgraph/first_subgraph/a\" [label=\"a\" ];\n"
        "            \"complex_graph/first_subgraph/first_subgraph/b\" [label=\"b\" ];\n"
        "        }\n"
        "        subgraph \"cluster_complex_graph/first_subgraph/second_subgraph\" {\n"
        "            label=\"second_subgraph\";\n"
        "            \"complex_graph/first_subgraph/second_subgraph/a\" [label=\"a\" ];\n"
        "            \"complex_graph/first_subgraph/second_subgraph/b\" [label=\"b\" ];\n"
        "        }\n"
        "    }\n"
        "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
        "        label=\"second_subgraph\";\n"
        "        subgraph \"cluster_complex_graph/second_subgraph/subgraph\" {\n"
        "            label=\"subgraph\";\n"
        "            \"complex_graph/second_subgraph/subgraph/a\" [label=\"a\" ];\n"
        "            \"complex_graph/second_subgraph/subgraph/b\" [label=\"b\" ];\n"
        "        }\n"
        "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    \"complex_graph/a\" [label=\"a\" ];\n"
        "    \"complex_graph/b\" [label=\"b\" ];\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/first_subgraph/first_subgraph/b\" "
        "[];\n"
        "    \"complex_graph/first_subgraph/second_subgraph/a\" -> "
        "\"complex_graph/first_subgraph/second_subgraph/b\" [];\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/first_subgraph/second_subgraph/b\" "
        "[];\n"
        "    \"complex_graph/second_subgraph/subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/a\" [];\n"
        "    \"complex_graph/second_subgraph/b\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
        "    \"complex_graph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "    \"complex_graph/b\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (EdgeWithAbsolutePath)
{
    std::stringstream stream;
    const bool exported = Graph::Export (EdgeWithAbsolutePath (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    label=\"complex_graph\";\n"
        "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
        "        label=\"first_subgraph\";\n"
        "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
        "        label=\"second_subgraph\";\n"
        "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
        "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (EdgeWithColor)
{
    std::stringstream stream;
    const bool exported = Graph::Export (EdgeWithColor (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"graph\" {\n"
        "    label=\"graph\";\n"
        "    \"graph/a\" [label=\"a\" ];\n"
        "    \"graph/b\" [label=\"b\" ];\n"
        "    \"graph/a\" -> \"graph/b\" [color=\"#FF0000FF\" ];\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (WithDuplicateGraphIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithDuplicateGraphIds (), stream);
    CHECK (!exported);
}

TEST_CASE (WithIncorrectGraphIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithIncorrectGraphIds (), stream);
    CHECK (!exported);
}

TEST_CASE (WithDuplicateNodeIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithDuplicateNodeIds (), stream);
    CHECK (!exported);
}

TEST_CASE (WithIncorrectNodeIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithIncorrectNodeIds (), stream);
    CHECK (!exported);
}

END_SUITE
