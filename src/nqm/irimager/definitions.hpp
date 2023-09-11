#ifndef NQM_IRIMAGER_DEFINITIONS
#define NQM_IRIMAGER_DEFINITIONS

#include <functional>
#include <string_view>

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
 * @warning This function must be thread-safe (it should use a mutex/lock).
 */
typedef std::function<void(LogLevel, std::string_view)> LoggingCallback;

#endif /* NQM_IRIMAGER_DEFINITIONS */
