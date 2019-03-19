/* Copyright (C) 2019 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EXTEND_LOW_INFO_GRAPH_HPP
#define EXTEND_LOW_INFO_GRAPH_HPP

#include "spatial_graph.hpp"
#include <functional>
namespace SG {

GraphType extend_low_info_graph_via_dfs(
    const std::vector<std::reference_wrapper<const GraphType>>& graphs,
    bool verbose = false);

} // end namespace SG
#endif
