#include "./irimager_class.hpp"

#include "spdlog/spdlog.h"

struct IRImager::impl {
 public:
  impl() = default;
  impl(const std::filesystem::path &xml_path) {
    // do a basic check that the given file is readable, and is an XML file
    auto xml_stream = std::ifstream(xml_path, std::fstream::in);
    auto xml_header = std::string(5, '\0');

    xml_stream.read(xml_header.data(),
                    static_cast<std::streamsize>(xml_header.size()));
    if (xml_header != std::string("<?xml")) {
      throw std::runtime_error(
          "Invalid XML file: The given XML file does not start with '<?xml'");
    }
  }

  /** @copydoc IRImager::start_streaming() */
  virtual void start_streaming() { streaming = true; }

  /** @copydoc IRImager::stop_streaming() */
  virtual void stop_streaming() { streaming = false; }

  /** @copydoc IRImager::get_frame() */
  virtual std::tuple<pybind11::array_t<uint16_t>,
                     std::chrono::system_clock::time_point>
  get_frame() {
    if (!streaming) {
      throw std::runtime_error("IRIMAGER_STREAMOFF: Not streaming");
    }

    auto frame_size = std::array<ssize_t, 2>{382, 288};
    auto my_array = pybind11::array_t<uint16_t>(frame_size);

    auto r = my_array.mutable_unchecked<frame_size.size()>();

    for (ssize_t i = 0; i < frame_size[0]; i++) {
      for (ssize_t j = 0; j < frame_size[1]; j++) {
        r(i, j) = static_cast<uint16_t>((1800 + 100) *
                                        std::pow(10, get_temp_range_decimal()));
      }
    }

    return std::make_tuple(my_array, std::chrono::system_clock::now());
  }

  /** @copydoc IRImager::get_temp_range_decimal() */
  virtual short get_temp_range_decimal() { return 1; }

  /** @copydoc IRImager::get_library_version() */
  virtual std::string_view get_library_version() = 0;

  virtual ~impl() = default;

 protected:
  bool streaming = false;
};

/**
 * Mocked implentation, doesn't use irimager or a real camera.
 */
struct IRImagerMockImpl final : public IRImager::impl {
 public:
  IRImagerMockImpl() { spdlog::warn("Creating a MOCKED IRImager object!"); }
  IRImagerMockImpl(const IRImager::impl &other) : IRImager::impl(other) {
    spdlog::warn("Creating a MOCKED IRImager object!");
  }
  IRImagerMockImpl(const std::filesystem::path &xml_path)
      : IRImager::impl(xml_path) {
    spdlog::warn("Creating a MOCKED IRImager object!");
  }

  std::string_view get_library_version() override { return "MOCKED"; }

  virtual ~IRImagerMockImpl() = default;
};

#ifdef IR_IMAGER_MOCK
#define IRImagerDefaultImplementation IRImagerMockImpl
#else  // not mocked

#include "libirimager/IRDevice.h"
#include "libirimager/IRImager.h"

/**
 * Real implentation, requires both a working irimager library, camera, and
 * calibration data.
 */
struct IRImagerRealImpl final : public IRImager::impl {
 public:
  IRImagerRealImpl() = default;
  IRImagerRealImpl(const IRImager::impl &other) : IRImager::impl(other) {}
  IRImagerRealImpl(const std::filesystem::path &xml_path)
      : IRImager::impl(xml_path) {}
  virtual ~IRImagerRealImpl() = default;

  std::string_view get_library_version() override {
    return evo::IRImager::getVersion();
  }

 private:
  std::shared_ptr<evo::IRDevice> ir_device;
};

#define IRImagerDefaultImplementation IRImagerRealImpl
#endif /* ifdef IR_IMAGER_MOCK */

IRImager::IRImager() = default;

IRImager::IRImager(const IRImager &other)
    : pImpl{std::make_unique<IRImagerDefaultImplementation>(*other.pImpl)} {}

IRImager::IRImager(IRImager &&other) = default;

IRImager::IRImager(const std::filesystem::path &xml_path)
    : pImpl{std::make_unique<IRImagerDefaultImplementation>(xml_path)} {}

IRImager::~IRImager() = default;

void IRImager::start_streaming() { pImpl->start_streaming(); }

void IRImager::stop_streaming() { pImpl->stop_streaming(); }

IRImager *IRImager::_enter_() {
  start_streaming();
  return this;
}

void IRImager::_exit_(
    [[maybe_unused]] const std::optional<pybind11::type> &exc_type,
    [[maybe_unused]] const std::optional<pybind11::object> &exc_value,
    [[maybe_unused]] const std::optional<pybind11::object> &traceback) {
  stop_streaming();
}

std::tuple<pybind11::array_t<uint16_t>, std::chrono::system_clock::time_point>
IRImager::get_frame() {
  return pImpl->get_frame();
}

short IRImager::get_temp_range_decimal() { return 1; }

std::string_view IRImager::get_library_version() {
  return pImpl->get_library_version();
}

IRImagerMock::IRImagerMock(const std::filesystem::path &xml_path) {
  pImpl = std::make_unique<IRImagerMockImpl>(xml_path);
}
