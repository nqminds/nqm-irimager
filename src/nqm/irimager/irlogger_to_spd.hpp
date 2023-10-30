#ifndef NQM_IRIMAGER_IRLOGGER_TO_SPD
#define NQM_IRIMAGER_IRLOGGER_TO_SPD

#include <filesystem>
#include <memory>

#include <propagate_const.h>

#include "./definitions.hpp"

/**
 * Handles capturing evo::IRLogger logs and passing them to a C++ callback.
 *
 * Since the only API that evo::IRLogger makes public is through a file,
 * we use a FIFO file socket to capture the data.
 *
 * @warning Creating multiple instances of IRLoggerToSpd is undefined behaviour.
 *          I haven't tested it, and the official evo::IRLogger docs say nothing
 *          about it, so your milage may vary.
 */
class IRLoggerToSpd {
 public:
  /**
   * Creates an IRLoggerToSpd using a random named socket.
   *
   * - The socket is placed into `$XDG_RUNTIME_DIR/nqm-irimager/` if it exists.
   * - Otherwise, an `{std::filesystem::temp_directory_path()}/nqm-irimager/`
   *   is used.
   */
  IRLoggerToSpd(const LoggingCallback &logging_callback);
  /**
   * Creates an IRLoggerToSpd using a socket on the given path.
   */
  IRLoggerToSpd(const LoggingCallback &logging_callback,
                const std::filesystem::path &socket_path);

  virtual ~IRLoggerToSpd();

  /** pImpl implementation */
  struct impl;

 private:
  // pImpl, see https://en.cppreference.com/w/cpp/language/pimpl
  std::experimental::fundamentals_v2::propagate_const<std::unique_ptr<impl>>
      pImpl;
};

#endif /** NQM_IRIMAGER_IRLOGGER_TO_SPD */
