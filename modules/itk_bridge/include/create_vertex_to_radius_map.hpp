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

#ifndef SG_CREATE_VERTEX_TO_RADIUS_MAP_HPP
#define SG_CREATE_VERTEX_TO_RADIUS_MAP_HPP

#include "image_types.hpp" // for image types
#include "spatial_graph.hpp"

#include <unordered_map>

namespace SG {

using VertexToRadiusMap =
        std::unordered_map<typename GraphType::vertex_descriptor, double>;

/**
 * Create a vertex to local radius map from a distance map and a graph.
 *
 * @param distance_map_image obtained from a binary image @sa
 * create_distance_map_function
 * @param input_graph input spatial graph to get the vertices/nodes
 * @param spatial_nodes_position_are_in_physical_space flag to check if
 *  position of nodes were already converted to physical space, or are still in index space.
 *  false indicates that positions are in index space. See @sa transform_to_physical_point
 * @param verbose extra information at execution
 *
 * @return vertex to local radius map using the distance map
 */
VertexToRadiusMap create_vertex_to_radius_map(
        const typename FloatImageType::Pointer &distance_map_image,
        const GraphType &input_graph,
        const bool spatial_nodes_position_are_in_physical_space = false,
        const bool verbose = false);

} // end namespace SG
#endif