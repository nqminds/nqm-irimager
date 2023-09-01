#ifndef NQM_IRIMAGER_LOGGER
#define NQM_IRIMAGER_LOGGER

#include <memory>
#include <string_view>

#include <pybind11/pybind11.h>

#include "./definitions.hpp"

/**
 * Handles converting C++ logs to Python :py:class:`logging.Logger`.
 *
 * After you instantiate an object of this class, all spdlogs will no longer
 * be printed to ``stderr``. Instead, they'll go to callback you've defined,
 * or a :py:class:`logging.Logger`.
 *
 * Additionally, evo::IRLogger logs will also be captured.
 *
 * Only a single instance of this object can exist at a time.
 * You must destroy existing instances to create a new instance.
 */
class Logger {
 public:
  /**
   * Creates a new logger with a custom logging callback.
   */
  Logger(LoggingCallback logging_callback);

  /**
   * Creates a new logger using a custom Python :py:class:`logging.Logger`
   * object
   */
  Logger(pybind11::object logger);

  /**
   * Creates a new logger using the default Python :py:class:`logging.Logger`
   */
  Logger();

  virtual ~Logger();

  /** pImpl implementation */
  struct impl;

 private:
  // pImpl, see https://en.cppreference.com/w/cpp/language/pimpl
  std::shared_ptr<impl> pImpl;
};

#endif /* NQM_IRIMAGER_LOGGER */
