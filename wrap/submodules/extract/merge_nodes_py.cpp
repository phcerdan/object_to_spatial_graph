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

#include "pybind11_common.h"
// #include "sgextract_common_py.hpp"
#include "merge_nodes.hpp"

namespace py = pybind11;
using namespace SG;

void init_merge_nodes(py::module &m) {
    m.def("merge_three_connected_nodes", &merge_three_connected_nodes);
    m.def("merge_four_connected_nodes", &merge_four_connected_nodes);
    m.def("merge_two_three_connected_nodes", &merge_two_three_connected_nodes);
    m.def("remove_parallel_edges", &remove_parallel_edges,
          R"(
Use @ref get_parallel_edges to remove the edges of the input graph.
Returns a new copy of the graph.

Parameters:
----------
graph: GraphType
 input spatial graph
keep_larger_spatial_edge: Bool [false by default]
 keep the parallel edge with largest contour length,
 if false, keep shorter parallel edges.
)",
          py::arg("graph"), py::arg("keep_larger_spatial_edge") = false);
}
