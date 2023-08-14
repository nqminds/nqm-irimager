#ifndef NQM_IRIMAGER_IRIMAGER
#define NQM_IRIMAGER_IRIMAGER

#include <Eigen/Dense>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "propagate_const.h"

/**
 * IRImager object - interfaces with a camera.
 */
class IRImager {
 public:
  /**
   * Thermal frame matrix.
   *
   * Uses Row-major order for better compatibility with Numpy.
   */
  using ThermalFrame =
      Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

  /**
   * Copies and existing IRImager object.
   */
  IRImager(const IRImager &);
  /**
   * Moves an existing IRImager object into the new stack location.
   */
  IRImager(IRImager &&);

  /**
   * Loads the configuration for an IR Camera from the given XML file
   *
   * @throw std::runtime_error if the XML file could not be read.
   */
  IRImager(const std::filesystem::path &xml_path);

  /**
   * @copydoc IRImager::IRImager(const std::filesystem::path &xml_path)
   *
   * @remark You should use
   * IRImager::IRImager(const std::filesystem::path &xml_path)
   * if possible. This function only exists to work-around C++11 libstdc++
   * ABI issues.
   */
  [[
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(gnu::access)
      gnu::access(read_only, 2, 3),
#endif
#endif
      gnu::nonnull(2)]] IRImager(const char *xml_path,
                                 std::size_t xml_path_len);

  /** Destructor */
  virtual ~IRImager();

  /**
   * Start video grabbing
   *
   * Prefer using `with irimager: ...` to automatically start/stop streaming
   * on errors.
   *
   * @throws RuntimeError if streaming cannot be started, e.g. if the camera
   *                      is not connected.
   */
  void start_streaming();

  /**
   * Stop video grabbing
   */
  void stop_streaming();

  /**
   * Return a frame.
   *
   * If the shutter is down
   * (normally done automatically by the thermal camera for calibration),
   * this function will wait until the shutter is back up, before returning
   * (usually around ~1s).
   *
   * @throws RuntimeError if a frame cannot be loaded,
   *                      e.g. if the camera isn't streaming.
   *
   * @returns A tuple containing:
   *         1. A 2-D matrix containing the image. This must be adjusted
   *           by :py:meth:`~IRImager.get_temp_range_decimal` to get the
   *           actual temperature in degrees Celcius, offset from -100 ℃.
   *         2. The time the image was taken.
   */
  std::tuple<ThermalFrame, std::chrono::system_clock::time_point> get_frame();

  /**
   * The number of decimal places in the thermal data
   *
   * For example, if :py:meth:`~IRImager.get_frame` returns 19000, you can
   * divide this number by 10 to the power of the result of
   * :py:meth:`~IRImager.get_temp_range_decimal`, then subtract 100,
   * to get the actual temperature in degrees Celcius.
   */
  short get_temp_range_decimal();

  /**
   * Get the version of the libirimager library.
   *
   * @returns the version of the libirmager library, or "MOCKED" if the
   * library has been mocked.
   */
  std::string_view get_library_version();

  /** pImpl implementation */
  struct impl;

 protected:
  // pImpl, see https://en.cppreference.com/w/cpp/language/pimpl
  std::experimental::fundamentals_v2::propagate_const<std::unique_ptr<impl>>
      pImpl;

  /**
   * Uninitialized constructor, should only be used in inheritance.
   */
  IRImager();
};

/**
 * Mocked version of IRImager.
 *
 * This class can be used to return dummy data when there isn't a camera
 * connected (e.g. for testing).
 */
class IRImagerMock : public IRImager {
 public:
  IRImagerMock(const std::filesystem::path &xml_path);

  /**
   * @copydoc IRImagerMock::IRImagerMock(const std::filesystem::path &xml_path)
   *
   * @remark You should use
   * IRImagerMock::IRImagerMock(const std::filesystem::path &xml_path)
   * if possible. This function only exists to work-around C++11 libstdc++
   * ABI issues.
   */
  [[
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(gnu::access)
      gnu::access(read_only, 2, 3),
#endif
#endif
      gnu::nonnull(2)]] IRImagerMock(const char *xml_path,
                                     std::size_t xml_path_len);
};

#endif /* NQM_IRIMAGER_IRIMAGER */
