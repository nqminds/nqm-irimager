#ifndef NQM_IRIMAGER_LOGGER_CONTEXT_MANAGER
#define NQM_IRIMAGER_LOGGER_CONTEXT_MANAGER

#include <memory>

#include <pybind11/pybind11.h>

#include "./logger.hpp"

/**
 * Context Manager around a Logger object.
 *
 * Designed for use with Python's ``with`` syntax.
 */
class LoggerContextManager {
 public:
  virtual ~LoggerContextManager();

  /**
   * Starts the logger if it's not already started.
   */
  void start();

  /** Stops the logger */
  void stop();

 private:
  std::unique_ptr<Logger> logger_;
};

#endif /* NQM_IRIMAGER_LOGGER_CONTEXT_MANAGER */
