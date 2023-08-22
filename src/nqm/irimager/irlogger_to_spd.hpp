#include <filesystem>
#include <memory>

#include "propagate_const.h"

class IRLoggerToSpd {
 public:
  /**
   * Creates an IRLoggerToSpd using a random named socket.
   *
   * - The socket is placed into `$XDG_RUNTIME_DIR/nqm-irimager/` if it exists.
   * - Otherwise, an `{std::filesystem::temp_directory_path()}/nqm-irimager/`
   *   is used.
   */
  IRLoggerToSpd();
  /**
   * Creates an IRLoggerToSpd using a socket on the given path.
   */
  IRLoggerToSpd(const std::filesystem::path &socket_path);

  virtual ~IRLoggerToSpd();

  /** pImpl implementation */
  struct impl;

 private:
  // pImpl, see https://en.cppreference.com/w/cpp/language/pimpl
  std::experimental::fundamentals_v2::propagate_const<std::unique_ptr<impl>>
      pImpl;
};
