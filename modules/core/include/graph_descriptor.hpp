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
#ifndef GRAPH_DESCRIPTOR_HPP
#define GRAPH_DESCRIPTOR_HPP

#include "spatial_graph.hpp"

namespace SG {

/**
 * Structure holding GraphType descriptors to localize a geometrical point in
 * the graph. The point can be in a node: vertex_descriptor, or in an edge:
 * edge_d + edge_points_index This graph_descriptor is used to map a vtk point
 * to differents graphs.
 */
struct graph_descriptor {
    /** the point exist in the graph */
    bool exist = false;
    /** the point is in an edge of the graph */
    bool is_edge = false;
    /** the point is in a vertex of the graph */
    bool is_vertex = false;
    /** vertex_descriptor where point is located */
    GraphType::vertex_descriptor vertex_d;
    /** edge_descriptor where point is located */
    GraphType::edge_descriptor edge_d;
    /** index of the edge_points vector where point is located inside the edge
     */
    std::size_t edge_points_index = std::numeric_limits<size_t>::max();
};

inline void
print_graph_descriptor(const graph_descriptor &descriptor,
                       const std::string &label = "graph_descriptor",
                       std::ostream &os = std::cout) {
    os << label << ":" << std::endl;
    if (!descriptor.exist) {
        os << " exist: NO" << std::endl;
        return;
    }
    if (descriptor.is_vertex) {
        os << " is_vertex" << std::endl;
        os << " vertex_d: " << descriptor.vertex_d << std::endl;
    }
    if (descriptor.is_edge) {
        os << " is_edge" << std::endl;
        os << " edge_d: " << descriptor.edge_d << std::endl;
        os << " edge_points_index: " << descriptor.edge_points_index
           << std::endl;
    }
    if (descriptor.is_vertex && descriptor.is_edge) {
        os << " WARNING: graph_descriptor is_vertex and is_edge at the same "
              "time."
           << std::endl;
    }
}

} // namespace SG
#endif
