#pragma once

#include <StandardLayout/Mapping.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::StandardLayout::Visualization
{
/// \brief StandardLayout::Mapping graph should have node with this name, that has edges with all direct fields.
constexpr const char *MAPPING_ROOT_NODE = ".";

/// \brief Creates visual graph from given mapping.
/// \details
/// - Graph id is guarantee to be equal to mapping name.
/// - Node VisualGraph::Common::Constants::DEFAULT_MAPPING_ROOT_NODE is guaranteed to exist.
/// - For each field, node with id, that is equal to field name, is guaranteed to exist.
VisualGraph::Graph GraphFromMapping (const Mapping &_mapping);
} // namespace Emergence::StandardLayout::Visualization
