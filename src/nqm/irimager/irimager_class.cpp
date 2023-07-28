#include "./irimager_class.hpp"

IRImager::IRImager() = default;

IRImager::IRImager(const std::filesystem::path &xml_path) {
    std::ifstream xml_stream(xml_path, std::fstream::in);

    std::string xml_header(5, '\0');
    xml_stream.read(xml_header.data(), xml_header.size());
    if (xml_header != std::string("<?xml")) {
        throw std::runtime_error("Invalid XML file: The given XML file does not start with '<?xml'");
    }
}

void IRImager::start_streaming() {
    streaming = true;
}

void IRImager::stop_streaming() {
    streaming = false;
}

IRImager* IRImager::_enter_() {
    start_streaming();
    return this;
}

void IRImager::_exit_(
    [[maybe_unused]] const std::optional<pybind11::type> &exc_type,
    [[maybe_unused]] const std::optional<pybind11::error_already_set> &exc_value,
    [[maybe_unused]] const pybind11::object &traceback) {

    stop_streaming();
}

std::tuple<
    pybind11::array_t<uint16_t>,
    std::chrono::system_clock::time_point
> IRImager::get_frame() {
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

short IRImager::get_temp_range_decimal() {
    return 1;
}
