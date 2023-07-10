#include <pybind11/pybind11.h>

class IRImager {
    public:
    int test() {
        return 42;
    }
};

PYBIND11_MODULE(irimager, m) {
    m.doc() = R"(Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.)";

    pybind11::class_<IRImager>(m, "IRImager", R"(IRImager object - interfaces with a camera.)")
        .def(pybind11::init<>())
        .def("test", &IRImager::test, "Return the number 42");
}
