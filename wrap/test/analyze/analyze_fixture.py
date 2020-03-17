# Copyright (C) 2019 Pablo Hernandez-Cerdan
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/

import _sgext.core as core

class AnalyzeGraphFixture:
    def __init__(self):
        self.graph = core.spatial_graph(6)
        # Set vertex positions
        v0 = self.graph.vertex(0);
        v0.pos = core.array.array3d(0,0,0)
        self.graph.set_vertex(0, v0)
        v1 = self.graph.vertex(1);
        v1.pos = core.array.array3d(1,1,0)
        self.graph.set_vertex(1, v1)
        v2 = self.graph.vertex(2);
        v2.pos = core.array.array3d(1,0,1)
        self.graph.set_vertex(2, v2)
        # Set edges between three nodes
        ed01 = core.spatial_edge()
        ed01.edge_points = [core.array.array3d(0.4, 0.6, 0)]
        core.graph.add_edge(0, 1, ed01, self.graph)
        ed02 = core.spatial_edge()
        ed02.edge_points = [core.array.array3d(0.4, 0, 0.6)]
        core.graph.add_edge(0, 2, ed02, self.graph)
        ed12 = core.spatial_edge()
        ed12.edge_points = [core.array.array3d(1, 0.4 , 0.6)]
        core.graph.add_edge(1, 2, ed12, self.graph)
        # Add end-points (degree 1)
        v3 = self.graph.vertex(3);
        v3.pos = core.array.array3d(-1,0,0)
        self.graph.set_vertex(3, v3)
        v4 = self.graph.vertex(4);
        v4.pos = core.array.array3d(1,2,0)
        self.graph.set_vertex(4, v4)
        v5 = self.graph.vertex(5);
        v5.pos = core.array.array3d(1,0,2)
        self.graph.set_vertex(5, v5)
        # Connect end-points
        core.graph.add_edge(0, 3, core.spatial_edge(), self.graph)
        core.graph.add_edge(1, 4, core.spatial_edge(), self.graph)
        core.graph.add_edge(2, 5, core.spatial_edge(), self.graph)
        # self.assertEqual(self.graph.num_vertices(), 6)
        # self.assertEqual(self.graph.num_edges(), 6)