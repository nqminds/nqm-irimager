#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>

#include <chrono>

class IRImager {
    public:
    int test() {
        return 42;
    }

    std::tuple<pybind11::array_t<uint16_t>, std::chrono::system_clock::time_point> get_frame() {
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
};

PYBIND11_MODULE(irimager, m) {
    m.doc() = R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)";

    pybind11::class_<IRImager>(m, "IRImager", R"(IRImager object - interfaces with a camera.)")
        .def(pybind11::init<>())
        .def("test", &IRImager::test, "Return the number 42")
        .def("get_frame", &IRImager::get_frame, R"(Return a frame

Returns:
    A tuple containing:
        - A 2-D numpy array containing the image.
        - The time the image was taken.
)");
}
