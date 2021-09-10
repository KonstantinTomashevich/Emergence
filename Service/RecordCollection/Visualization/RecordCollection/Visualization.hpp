#pragma once

#include <RecordCollection/Collection.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::RecordCollection::Visualization
{
/// \brief Collection graph is guaranteed to have node with this name, that will be used as root for its graph.
constexpr const char *COLLECTION_ROOT_NODE = ".";

/// \brief Representation graph is guaranteed to have node with this name, that will be used as root for its graph.
constexpr const char *REPRESENTATION_ROOT_NODE = ".";

/// \brief Creates visual graph from given collection.
/// \details
/// - Guaranteed to have COLLECTION_ROOT_NODE.
/// - Guaranteed to have nodes for every child representation.
VisualGraph::Graph GraphFromCollection (const RecordCollection::Collection &_collection);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromLinearRepresentation (const RecordCollection::LinearRepresentation &_representation);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromPointRepresentation (const RecordCollection::PointRepresentation &_representation);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromVolumetricRepresentation (
    const RecordCollection::VolumetricRepresentation &_representation);

/// \return Id of a graph, that could be created from given collection.
std::string GraphId (const RecordCollection::Collection &_collection);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const RecordCollection::LinearRepresentation &_representation);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const RecordCollection::PointRepresentation &_representation);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const RecordCollection::VolumetricRepresentation &_representation);
} // namespace Emergence::RecordCollection::Visualization
