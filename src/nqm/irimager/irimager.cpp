#include <nanobind/nanobind.h>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/filesystem.h>

#include "./irimager_class.hpp"

#ifndef DOCSTRINGS_H
#error DOCSTRINGS_H must be defined to the output of pybind11_mkdocs
#endif

#include DOCSTRINGS_H

NB_MODULE(irimager, m) {
    m.doc() = R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)";

    nanobind::class_<IRImager>(m, "IRImager", DOC(IRImager))
        .def(nanobind::init<>())
        .def(nanobind::init<const std::filesystem::path &>(), DOC(IRImager, IRImager, 2))
        .def("get_frame", &IRImager::get_frame, DOC(IRImager, get_frame))
        .def("get_temp_range_decimal", &IRImager::get_temp_range_decimal, DOC(IRImager, get_temp_range_decimal))
        .def("start_streaming", &IRImager::start_streaming, DOC(IRImager, start_streaming))
        .def("stop_streaming", &IRImager::stop_streaming, DOC(IRImager, stop_streaming))
        .def("__enter__", &IRImager::_enter_, nanobind::rv_policy::reference_internal)
        .def("__exit__", &IRImager::_exit_);
}
