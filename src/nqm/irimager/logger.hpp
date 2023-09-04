#ifndef NQM_IRIMAGER_LOGGER
#define NQM_IRIMAGER_LOGGER

#include <memory>
#include <string_view>

#include <pybind11/pybind11.h>

/**
 * Enum that converts C++ logs to Python log levels.
 *
 * These are copied from
 * https://docs.python.org/3/library/logging.html#logging-levels when possible.
 */
enum class LogLevel {
  trace = 5,
  debug = 10,
  info = 20,
  warn = 30,
  error = 40,
  critical = 50,
};

/**
 * @brief Logging callback function.
 *
 * @warning Please be aware that this std::function might have a Python object
 * inside of it.
 * This means that you **MUST NOT COPY OR DELETE** LoggingCallback objects,
 * **UNLESS** you have the Python GIL.
 */
typedef std::function<void(LogLevel, std::string_view)> LoggingCallback;

/**
 * Handles converting C++ spdlog to Python :py:class:`logging.Logger`.
 *
 * After you instantiate an object of this class, all spdlogs will no longer
 * be printed to ``stderr``. Instead, they'll go to callback you've defined,
 * or a :py:class:`logging.Logger`.
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
