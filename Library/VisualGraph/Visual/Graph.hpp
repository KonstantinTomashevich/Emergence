#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Emergence::VisualGraph
{
struct Node
{
    const std::string id;

    const std::optional<std::string> label;
};

struct Edge
{
    const std::string from;

    const std::string to;
};

struct Graph
{
    const std::string id;

    const std::optional<std::string> label;

    const std::vector<Graph> subgraphs;

    const std::vector<Node> nodes;

    const std::vector<Edge> edges;
};
} // namespace Emergence::VisualGraph
