#include "./irimager_class.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <variant>

#include <spdlog/spdlog.h>

#include "./chrono.hpp"

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
  virtual std::tuple<IRImager::ThermalFrame,
                     std::chrono::system_clock::time_point>
  get_frame() {
    if (!streaming) {
      throw std::runtime_error("IRIMAGER_STREAMOFF: Not streaming");
    }

    auto frame_size = std::array<ssize_t, 2>{382, 288};
    auto max_value = static_cast<uint16_t>(
        (1800 + 100) * std::pow(10, get_temp_range_decimal()));
    auto my_array = IRImager::ThermalFrame::Constant(frame_size[0],
                                                     frame_size[1], max_value);

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

#include <libirimager/IRDevice.h>
#include <libirimager/IRDeviceParams.h>
#include <libirimager/IRImager.h>
#include <libirimager/IRLogger.h>

/**
 * Exception class to convert evo::IRDeviceError to a human readable error.
 */
class IRDeviceException : public std::runtime_error {
 public:
  IRDeviceException(evo::IRDeviceError ir_device_error)
      : std::runtime_error(enum_to_string_message(ir_device_error)) {}

  static std::string enum_to_string_message(
      evo::IRDeviceError ir_device_error) {
    switch (ir_device_error) {
      case evo::IRIMAGER_SUCCESS:
        throw std::invalid_argument("IRIMAGER_SUCCESS is not an error");
      case evo::IRIMAGER_NODATA:
        return "IRIMAGER_NODATA: Error occurred in getting frame. You may need "
               "to reconnect the camera.";
      case evo::IRIMAGER_DISCONNECTED:
        return "IRIMAGER_DISCONNECTED: Error occurred in getting frame. You "
               "may need to reconnect the camera.";
      case evo::IRIMAGER_NOSYNC:
        return "IRIMAGER_NOSYNC: Error occurred in getting frame. You may need "
               "to reconnect the camera.";
      case evo::IRIMAGER_STREAMOFF:
        return "IRIMAGER_STREAMOFF: Not streaming";
      case evo::IRIMAGER_EAGAIN:
        return "IRIMAGER_EAGAIN: Error occurred in getting frame. You may need "
               "to reconnect the camera.";
      case evo::IRIMAGER_EIO:
        return "IRIMAGER_EIO: Error occurred in getting frame. You may need to "
               "reconnect the camera.";
      case evo::IRIMAGER_EUNKNOWN:
        return "IRIMAGER_EUNKNOWN: Error occurred in getting frame. You may "
               "need to reconnect the camera.";
    }

    return "IRImager::getFrame() returned an unknown error code";
  }
};

template <class, class = void>
struct has_get_temp_range_decimal : std::false_type {};

/**
 * True if the given class has a `get_temp_range_decimal()` method.
 */
template <class T>
struct has_get_temp_range_decimal<
    T, std::void_t<decltype(std::declval<T>().get_temp_range_decimal())> >
    : std::true_type {};

/**
 * Real implentation, requires both a working irimager library, camera, and
 * calibration data.
 */
struct IRImagerRealImpl final : public IRImager::impl {
 public:
  IRImagerRealImpl() = default;

  IRImagerRealImpl(const IRImager::impl &other) : IRImager::impl(other) {
    auto other_real = dynamic_cast<const IRImagerRealImpl *>(&other);
    if (other_real != nullptr) {
      ir_device = other_real->ir_device;
    }
  }

  IRImagerRealImpl(const std::filesystem::path &xml_path) {
    evo::IRDeviceParams params;
    if (evo::IRDeviceParamsReader::readXMLC(xml_path.c_str(), params)) {
      // do a basic check that the given file is readable, and is an XML file
      auto xml_stream = std::ifstream(xml_path, std::fstream::in);
      auto xml_header = std::string(5, '\0');
      xml_stream.read(xml_header.data(),
                      static_cast<std::streamsize>(xml_header.size()));
      if (xml_header != std::string("<?xml")) {
        throw std::runtime_error(
            "Invalid XML file: The given XML file does not start with '<?xml'");
      }

      // I'm not sure why, but readXMLC seems to always fail for me.
      // The IRImagerLogs don't show any warnings and ignoring the error seems
      // to work fine, so 🤷
      spdlog::warn("Ignoring error when reading XML file at {}.",
                   xml_path.string());
    }

    ir_device.reset(evo::IRDevice::IRCreateDevice(params));

    if (ir_device == nullptr) {
      throw std::runtime_error("Failed to create IRDevice");
    }

    ir_imager.init(&params, ir_device->getFrequency(), ir_device->getWidth(),
                   ir_device->getHeight(), ir_device->controlledViaHID());
    ir_imager.setThermalFrameCallback(&onThermalFrame);
  }

  virtual ~IRImagerRealImpl() = default;

  void start_streaming() override {
    // despite what the docs say, startStreaming() returns 0 on success, non-0
    // on failure
    if (ir_device->startStreaming() != 0) {
      throw std::runtime_error(
          "Error occurred in starting stream. You may need to reconnect the "
          "camera.");
    }
  }

  virtual void stop_streaming() override {
    // despite what the docs say, stopStreaming() returns 0 on success, non-0 on
    // failure
    if (ir_device->stopStreaming() != 0) {
      throw std::runtime_error("Error occurred in stopping stream.");
    }
  }

  std::tuple<IRImager::ThermalFrame, std::chrono::system_clock::time_point>
  get_frame() override {
    auto raw_frame_bytes =
        std::vector<unsigned char>(ir_device->getRawBufferSize());
    /** time of frame, in monotonic seconds since std::chrono::steady_clock */
    double timestamp;
    evo::IRDeviceError device_error =
        ir_device->getFrame(raw_frame_bytes.data(), &timestamp);
    if (device_error != evo::IRIMAGER_SUCCESS) {
      throw IRDeviceException(device_error);
    }

    ir_imager.process(raw_frame_bytes.data(), static_cast<void *>(this));

    std::variant<IRImager::ThermalFrame, BadFrame, nullptr_t>
        thermal_data_result = nullptr;
    {
      // wait until ir_imager.process() calls the
      // IRImager::impl::onThermalFrame callback and fills @p thermal_data
      auto lk = std::unique_lock(mutex);

      if (thermal_data_available.wait_for(lk, std::chrono::seconds(30), [&] {
            return !std::holds_alternative<nullptr_t>(thermal_data);
          }) == false) {
        throw std::runtime_error(
            "Timeout when waiting for a new thermal frame");
      }

      thermal_data_result.swap(thermal_data);
    }

    if (std::holds_alternative<BadFrame>(thermal_data_result)) {
      switch (std::get<BadFrame>(thermal_data_result)) {
        case BadFrame::SHUTTER_CLOSED:
          spdlog::debug("Shutter was down, trying to take a frame again");
#ifdef __clang__
          // GCC will tail-call optimize too on x86_64 and ARM64, but even if it
          // doesn't we're extremely unlikely to have a stack overflow, even if
          // imaging at 1000Hz
          [[clang::musttail]] return get_frame();
#else
          return get_frame();
#endif
      }

      throw std::runtime_error("Failed to get a thermal frame");
    }

    auto seconds_since_epoch =
        std::chrono::duration<double, std::ratio<1> >(timestamp);
    auto nanoseconds_since_epoch =
        std::chrono::floor<std::chrono::nanoseconds>(seconds_since_epoch);

    // need to convert our double duration to an integer duration
    auto steady_time_point = std::chrono::time_point<std::chrono::steady_clock>(
        nanoseconds_since_epoch);

    return std::make_tuple(
        std::get<IRImager::ThermalFrame>(std::move(thermal_data_result)),
        nqm::irimager::clock_cast(steady_time_point));
  }

  short get_temp_range_decimal() override {
    static_assert(has_get_temp_range_decimal<evo::IRImager>::value == false,
                  "evo::IRImager has a get_temp_range_decimal function, which "
                  "should be used instead of this override.");
    return 1;
  }

  std::string_view get_library_version() override {
    return evo::IRImager::getVersion();
  }

 private:
  std::shared_ptr<evo::IRDevice> ir_device;
  evo::IRImager ir_imager;

  /**
   * Enum that explains why a frame might be bad.
   */
  enum class BadFrame {
    /** The shutter was closed, so this frame is only for calibration */
    SHUTTER_CLOSED,
  };

  /**
   * Locks the ::thermal_data_available and ::thermal_data attributes.
   */
  std::mutex mutex;
  /**
   * Notifies that the ::thermal_data param holds data.
   */
  std::condition_variable thermal_data_available;
  /**
   * Either a:
   *   - IRImager::ThermalFrame, containing the thermal frame,
   *   - ::BadFrame, containing an enum on why the frame is bad,
   *   - or `nullptr`, which means there is no frame data.
   */
  std::variant<IRImager::ThermalFrame, BadFrame, nullptr_t> thermal_data =
      nullptr;

  /**
   * Thermal frame callback function.
   *
   * Stores the thermal frame in the ::thermal_data attribute, then sends a
   * notification on ::thermal_data_available to let any listeners know that
   * a frame has arrived.
   *
   * @param[in] thermal Thermal data 2D-array.
   * @param w           Width of thermal data array.
   * @param h           Height of thermal data array.
   * @param meta        Metadata about the frame.
   * @param[in] arg     Pointer passed to evo::IRImager::process().
   *                    In our code, this should always be a IRImager::impl
   *                    object.
   * @see evo::IRImager::setThermalFrameCallback(fptrIRThermalFrame callback)
   */
  static void onThermalFrame(unsigned short *thermal, unsigned int w,
                             unsigned int h,
                             [[maybe_unused]] evo::IRFrameMetadata meta,
                             void *arg) {
    auto data = static_cast<IRImagerRealImpl *>(arg);
    {
      auto lock = std::scoped_lock(data->mutex);

      if (!std::holds_alternative<nullptr_t>(data->thermal_data)) {
        spdlog::warn(
            "thermal_data is not empty, this might occur due to calling "
            "`get_frame()` before the previous `get_frame()` is finished");
      }

      if (data->ir_imager.isFlagOpen()) {
        // TODO: evo::IRImager doesn't mention if data is RowMajor/ColumnMajor
        //       so we're assuming it's RowMajor.
        auto array = Eigen::Map<IRImager::ThermalFrame>(thermal, w, h);

        data->thermal_data = std::move(array);
      } else {
        // shutter is down, frame data is bad
        data->thermal_data = BadFrame::SHUTTER_CLOSED;
      }
    }
    data->thermal_data_available.notify_one();
  }
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

std::tuple<IRImager::ThermalFrame, std::chrono::system_clock::time_point>
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
