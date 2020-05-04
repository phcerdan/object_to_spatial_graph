/* Copyright (C) 2019 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pybind11_common.h"

#include "contour_length_generator.hpp"
#include "contour_length_generator_functions.hpp"

namespace py = pybind11;
using namespace SG;

void init_contour_length_generator(py::module &m) {
    m.def("generate_contour_length",
           &generate_contour_length);
    py::class_<contour_length_generator>(m, "contour_length_generator")
            .def("generate_contour_length",
                 &contour_length_generator::generate_contour_length);
}
