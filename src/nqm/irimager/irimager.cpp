#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>

#include "./irimager_class.hpp"

#ifndef DOCSTRINGS_H
#error DOCSTRINGS_H must be defined to the output of pybind11_mkdocs
#endif

#ifndef SKBUILD_PROJECT_VERSION
#error SKBUILD_PROJECT_VERSION must be defined as the Python package version
#endif

#include DOCSTRINGS_H

PYBIND11_MODULE(irimager, m) {
    m.doc() = R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)";

    m.attr("__version__") = SKBUILD_PROJECT_VERSION;

    pybind11::class_<IRImager>(m, "IRImager", DOC(IRImager))
        .def(pybind11::init<>())
        .def(pybind11::init<const std::filesystem::path &>(), DOC(IRImager, IRImager, 2))
        .def("get_frame", &IRImager::get_frame, DOC(IRImager, get_frame))
        .def("get_temp_range_decimal", &IRImager::get_temp_range_decimal, DOC(IRImager, get_temp_range_decimal))
        .def("get_library_version", &IRImager::get_library_version, DOC(IRImager, get_library_version))
        .def("start_streaming", &IRImager::start_streaming, DOC(IRImager, start_streaming))
        .def("stop_streaming", &IRImager::stop_streaming, DOC(IRImager, stop_streaming))
        .def("__enter__", &IRImager::_enter_, pybind11::return_value_policy::reference_internal)
        .def("__exit__", &IRImager::_exit_);
}
