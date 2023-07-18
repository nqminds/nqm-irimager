#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <pybind11/numpy.h>

/**
 * IRImager object - interfaces with a camera.
 */
class IRImager {
    public:
    IRImager() = default;
    /**
     * Loads the configuration for an IR Camera from the given XML file
     */
    IRImager(const std::filesystem::path &xml_path) {
        std::ifstream xml_stream(xml_path, std::fstream::in);

        std::string xml_header(5, '\0');
        xml_stream.read(xml_header.data(), xml_header.size());
        if (xml_header != std::string("<?xml")) {
            throw std::runtime_error("Invalid XML file: The given XML file does not start with '<?xml'");
        }
    }

    /**
     * Start video grabbing
     *
     * Prefer using `with irimager: ...` to automatically start/stop streaming
     * on errors.
     *
     * @throws RuntimeError if streaming cannot be started, e.g. if the camera
     *                      is not connected.
     */
    void start_streaming() {
        streaming = true;
    }

    /**
     * Stop video grabbing
     */
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

    /**
     * Return a frame
     *
     * @throws RuntimeError if a frame cannot be loaded,
     *   e.g. if the camera isn't streaming.
     *
     * @returns A tuple containing:
     *         1. A 2-D numpy array containing the image. This must be adjusted
     *           by :py:meth:`~IRImager.get_temp_range_decimal` to get the
     *           actual temperature in degrees Celcius.
     *         2. The time the image was taken.
     */
    std::tuple<pybind11::array_t<uint16_t>, std::chrono::system_clock::time_point> get_frame() {
        if (!streaming) {
            throw std::runtime_error("IRIMAGER_STREAMOFF: Not streaming");
        }

        auto frame_size = std::array<ssize_t, 2>{128, 128};
        auto my_array = pybind11::array_t<uint16_t>(frame_size);

        auto r = my_array.mutable_unchecked<frame_size.size()>();

        for (ssize_t i = 0; i < frame_size[0]; i++) {
            for (ssize_t j = 0; j < frame_size[1]; j++) {
                r(i, j) = 1800 * std::pow(10, get_temp_range_decimal());
            }
        }

        return std::make_tuple(my_array, std::chrono::system_clock::now());
    }

    /**
     * The number of decimal places in the thermal data
     *
     * For example, if :py:meth:`~IRImager.get_frame` returns 18000, you can
     * divide this number by 10 to the power of the result of
     * :py:meth:`~IRImager.get_temp_range_decimal` to get the actual
     * temperature in degrees Celcius.
     */
    short get_temp_range_decimal() {
        return 1;
    }

private:
    bool streaming = false;
};
