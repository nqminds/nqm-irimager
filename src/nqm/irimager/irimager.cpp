#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

class IRImager {
    public:
    IRImager() = default;
    IRImager(const std::filesystem::path &xml_path) {
        std::ifstream xml_stream(xml_path, std::fstream::in);

        std::string xml_header(5, '\0');
        xml_stream.read(xml_header.data(), xml_header.size());
        if (xml_header != std::string("<?xml")) {
            throw std::runtime_error("Invalid XML file: The given XML file does not start with '<?xml'");
        }
    }

    int test() {
        return 42;
    }

    void start_streaming() {
        streaming = true;
    }

    void stop_streaming() {
        streaming = false;
    }

    IRImager* _enter_() {
        start_streaming();
        return this;
    }

    void _exit_(
        [[maybe_unused]] const std::optional<pybind11::type> &exc_type,
        [[maybe_unused]] const std::optional<pybind11::error_already_set> &exc_value,
        [[maybe_unused]] const pybind11::object &traceback) {
        stop_streaming();
    }

    std::tuple<pybind11::array_t<uint16_t>, std::chrono::system_clock::time_point> get_frame() {
        if (!streaming) {
            throw std::runtime_error("IRIMAGER_STREAMOFF: Not streaming");
        }

        auto frame_size = std::array<ssize_t, 2>{128, 128};
        auto my_array = pybind11::array_t<uint16_t>(frame_size);

        auto r = my_array.mutable_unchecked<frame_size.size()>();

        for (ssize_t i = 0; i < frame_size[0]; i++) {
            for (ssize_t j = 0; j < frame_size[1]; j++) {
                r(i, j) = 1800;
            }
        }

        return std::make_tuple(my_array, std::chrono::system_clock::now());
    }

private:
    bool streaming = false;
};

PYBIND11_MODULE(irimager, m) {
    m.doc() = R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)";

    pybind11::class_<IRImager>(m, "IRImager", R"(IRImager object - interfaces with a camera.)")
        .def(pybind11::init<>())
        .def(pybind11::init<const std::filesystem::path &>(), R"(Loads the configuration for an IR Camera from the given XML file)")
        .def("test", &IRImager::test, "Return the number 42")
        .def("get_frame", &IRImager::get_frame, R"(Return a frame

Raises:
    RuntimeError: If a frame cannot be loaded, e.g. if the camera isn't streaming.

Returns:
    A tuple containing:
        - A 2-D numpy array containing the image.
        - The time the image was taken.
)")
        .def("start_streaming", &IRImager::start_streaming, R"(Start video grabbing

Prefer using `with irimager: ...` to automatically start/stop streaming on errors.

Raises:
    RuntimeError: If streaming cannot be started, e.g. if the camera is not connected.
)")
        .def("stop_streaming", &IRImager::stop_streaming, R"(Stop video grabbing)")
        .def("__enter__", &IRImager::_enter_, pybind11::return_value_policy::reference_internal)
        .def("__exit__", &IRImager::_exit_);
}
