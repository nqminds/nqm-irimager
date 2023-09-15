#ifndef NQM_IRIMAGER_IRIMAGER
#define NQM_IRIMAGER_IRIMAGER

#include <pybind11/numpy.h>

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
   * Copies and existing IRImager object.
   */
  IRImager(const IRImager &);
  /**
   * Moves an existing IRImager object into the new stack location.
   */
  IRImager(IRImager &&);

  /**
   * Loads the configuration for an IR Camera from the given XML file
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
  IRImager(const char *xml_path, std::size_t xml_path_len);

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

  IRImager *_enter_();

  void _exit_(const std::optional<pybind11::type> &exc_type,
              const std::optional<pybind11::object> &exc_value,
              const std::optional<pybind11::object> &traceback);

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
  std::tuple<pybind11::array_t<uint16_t>, std::chrono::system_clock::time_point>
  get_frame();

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
};

#endif /* NQM_IRIMAGER_IRIMAGER */
