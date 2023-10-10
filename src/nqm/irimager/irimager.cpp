#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
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

  // helps prevent deadlock when calling code that doesn't touch Python objs
  const auto no_gil = pybind11::call_guard<pybind11::gil_scoped_release>();

  pybind11::class_<IRImager>(m, "IRImager", DOC(IRImager))
      .def(pybind11::init<const std::filesystem::path &>(),
           DOC(IRImager, IRImager), no_gil)
      .def("get_frame", &IRImager::get_frame,
           DOC(IRImager, get_frame))  // get_frame needs GIL
      .def("get_temp_range_decimal", &IRImager::get_temp_range_decimal,
           DOC(IRImager, get_temp_range_decimal), no_gil)
      .def("get_library_version", &IRImager::get_library_version,
           DOC(IRImager, get_library_version), no_gil)
      .def("start_streaming", &IRImager::start_streaming,
           DOC(IRImager, start_streaming), no_gil)
      .def("stop_streaming", &IRImager::stop_streaming,
           DOC(IRImager, stop_streaming), no_gil)
      .def("__enter__", &IRImager::_enter_,
           pybind11::return_value_policy::reference_internal, no_gil)
      .def("__exit__", &IRImager::_exit_);

  pybind11::class_<IRImagerMock, IRImager>(m, "IRImagerMock", DOC(IRImagerMock))
      .def(pybind11::init<const std::filesystem::path &>(),
           DOC(IRImager, IRImager), no_gil)
      .def("get_frame", &IRImagerMock::get_frame,
           DOC(IRImager, get_frame))  // get_frame needs GIL
      .def("get_temp_range_decimal", &IRImagerMock::get_temp_range_decimal,
           DOC(IRImager, get_temp_range_decimal), no_gil)
      .def("start_streaming", &IRImagerMock::start_streaming,
           DOC(IRImager, start_streaming), no_gil)
      .def("stop_streaming", &IRImagerMock::stop_streaming,
           DOC(IRImager, stop_streaming), no_gil)
      .def("__enter__", &IRImagerMock::_enter_,
           pybind11::return_value_policy::reference_internal, no_gil)
      .def("__exit__", &IRImagerMock::_exit_);
}
