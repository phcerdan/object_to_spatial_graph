/* ********************************************************************
 * Copyright (C) 2020 Pablo Hernandez-Cerdan.
 *
 * This file is part of SGEXT: http://github.com/phcerdan/sgext.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * *******************************************************************/

#ifndef FILTER_SPATIAL_GRAPH_HPP
#define FILTER_SPATIAL_GRAPH_HPP

#include "bounding_box.hpp"
#include "hash_edge_descriptor.hpp"
#include "spatial_graph.hpp"
#include <boost/graph/filtered_graph.hpp>

// Create a filtered_graph type, use keep tag
// Create bool function for edges and vertices to filter. Return true if in
// bounding box. Function to apply this to a graph given a bounding box.
//  -- With option to just view (the default)
//  -- Or to create a  new graph from the view. This has the problem that the
//  ids of the filtered graph and the original graph might differ.
//
//  The bounding box should be in index or physical coordinates space?
//  Extras needed: A map with keys in bounding box coordinates and values in
//  graph ids, edge and/or vertex.
//

namespace SG {

using FilteredGraphType = boost::filtered_graph<
        GraphType,
        std::function<bool(GraphType::edge_descriptor)>,
        std::function<bool(GraphType::vertex_descriptor)>>;
using ComponentGraphType = FilteredGraphType;

using VertexDescriptorUnorderedSet =
        std::unordered_set<GraphType::vertex_descriptor>;
using EdgeDescriptorUnorderedSet =
        std::unordered_set<GraphType::edge_descriptor,
                           SG::edge_hash<GraphType>>;

FilteredGraphType filter_by_bounding_box_no_copy(const BoundingBox &box,
                                                 const GraphType &g);
/**
 * Return a new graph which is inside the bounding box. Please note that indices
 * or ids are unrelated to the input graph.
 *
 * Nodes might exist outside the bounding box if any edge crosses the boundary.
 *
 * XXX: Please note that this operation is pure brute force and quite
 * expensive.
 * Better (if possible) to clip the thin image, and compute graph from it.
 *
 * @param box the bounding box in the same coordinates than the spatial graph
 * @param g spatial graph
 *
 * @return a copy of the graph
 */
GraphType filter_by_bounding_box(const BoundingBox &box, const GraphType &g);

FilteredGraphType
filter_by_sets_no_copy(const EdgeDescriptorUnorderedSet &remove_edges,
                       const VertexDescriptorUnorderedSet &remove_nodes,
                       const GraphType &g);
GraphType filter_by_sets(const EdgeDescriptorUnorderedSet &remove_edges,
                         const VertexDescriptorUnorderedSet &remove_nodes,
                         const GraphType &g);

/**
 * Return all the different components of the input graph as a vector
 * of filtered graphs.
 *
 * Filtered graphs are views of the original graph, they share the same
 * vertex and edge descriptors.
 * However, num_edges and num_vertices of a filtered graph returns the numbers
 * on the original graph. Note that there is no copy, so the inputGraph has to
 * outlive the filtered graph
 *
 * @param inputGraph
 *
 * @return
 */
std::vector<ComponentGraphType>
filter_component_graphs(const GraphType &inputGraph);
std::vector<ComponentGraphType> filter_component_graphs(
        const GraphType &inputGraph,
        const size_t num_of_components,
        const std::unordered_map<GraphType::vertex_descriptor, int>
                &components_map);

/**
 * Create a new graph holding the largest component of the input graph.
 *
 * If you want to work with filtered_graph, use @sa filter_component_graphs
 * Please note that after the copy, the returned graph won't share vertex or
 * edge indices with the original graph. Use the filtered_graph interface
 * if you need them.
 *
 * @param inputGraph input spatial graph
 *
 * @return largest component graph
 */
GraphType copy_largest_connected_component(const GraphType &inputGraph);


/**
 * Append g_to_add to g_out in place (g_out will be modified).
 *
 * @param g_out
 * @param g_to_add
 */
void append_graph_in_place(GraphType & g_out, const GraphType & g_to_add);
} // namespace SG
#endif
