#include "./irimager_class.hpp"

#include "spdlog/spdlog.h"

struct IRImager::impl {
 public:
  impl() = default;
  impl(const std::filesystem::path &xml_path) {
    // do a basic check that the given file is readable, and is an XML file
    auto xml_stream = std::ifstream(xml_path, std::fstream::in);

    try {
      xml_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    } catch (const std::ios_base::failure &error) {
      // std::ios_base::failure isn't a std::runtime_error,
      // if we're compiling with _GLIBCXX_USE_CXX11_ABI=0
      throw std::runtime_error(std::string("Failed to open file ") +
                               xml_path.string() + " due to " + error.what());
    }

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
  virtual std::tuple<Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>,
                     std::chrono::system_clock::time_point>
  get_frame() {
    if (!streaming) {
      throw std::runtime_error("IRIMAGER_STREAMOFF: Not streaming");
    }

    auto frame_size = std::array<ssize_t, 2>{382, 288};
    auto max_value = static_cast<uint16_t>(
        (1800 + 100) * std::pow(10, get_temp_range_decimal()));
    auto my_array =
        Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>::Constant(
            frame_size[0], frame_size[1], max_value);

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

IRImager::IRImager(const char *xml_path, std::size_t xml_path_len)
    : IRImager(std::string(xml_path, xml_path_len)) {}

IRImager::~IRImager() = default;

void IRImager::start_streaming() { pImpl->start_streaming(); }

void IRImager::stop_streaming() { pImpl->stop_streaming(); }

std::tuple<Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>,
           std::chrono::system_clock::time_point>
IRImager::get_frame() {
  return pImpl->get_frame();
}

short IRImager::get_temp_range_decimal() {
  return pImpl->get_temp_range_decimal();
}

std::string_view IRImager::get_library_version() {
  return pImpl->get_library_version();
}

IRImagerMock::IRImagerMock(const std::filesystem::path &xml_path) {
  pImpl = std::make_unique<IRImagerMockImpl>(xml_path);
}

IRImagerMock::IRImagerMock(const char *xml_path, std::size_t xml_path_len)
    : IRImagerMock(std::string(xml_path, xml_path_len)) {}
