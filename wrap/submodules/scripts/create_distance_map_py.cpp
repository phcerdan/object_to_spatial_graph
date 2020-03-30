/* Copyright (C) 2019 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "create_distance_map_function.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace SG;
void init_create_distance_map(py::module &m) {
    m.def("create_distance_map", &create_distance_map_function,
            "Create distance map using DGtal (high precision). Returns dist_map image.",
            py::arg("input"),
            py::arg("verbose") = false
         );
    m.def("create_distance_map_io", &create_distance_map_function_io,
            "Create distance map using DGtal (high precision). Read/write from/to file.",
            py::arg("input_file"),
            py::arg("out_folder"),
            py::arg("foreground") = "white",
            py::arg("verbose") = false
         );

}