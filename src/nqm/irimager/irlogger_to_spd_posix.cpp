#include "./irlogger_to_spd.hpp"

#if __has_include(<unistd.h>)
// this is fine!! Expected behavior
#else
#error \
    "This file requires OS functions that are only available on POSIX systems"
#endif

extern "C" {
#include <unistd.h>
}

#ifndef _POSIX_VERSION
#error \
    "This file requires OS functions that are only available on POSIX systems"
#endif

#include <atomic>
#include <cerrno>  // POSIX errno
#include <exception>
#include <future>
#include <memory>

extern "C" {
// needed for the mkfifo() command
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
}

#include <pybind11/pybind11.h>
#include <spdlog/spdlog.h>

#include "./chrono.hpp"
#include "./irlogger_parser.hpp"

/** RAII wrapper to handle reading from a POSIX FIFO */
class PosixFileReadOnly {
 public:
  PosixFileReadOnly(const std::filesystem::path &_path) : path{_path} {
    posix_file_descriptor = open(path.string().c_str(), O_RDONLY | O_NONBLOCK);
    if (posix_file_descriptor == -1) {
      auto exception =
          std::system_error(std::error_code(errno, std::system_category()));
      spdlog::error("Failed to open file at {} due to {}", path.string(),
                    exception.what());
      throw exception;
    }
  }

  /**
   * Tries reading some bytes from the file without blocking.
   *
   * @param[out] buffer The buffer to store data in.
   * @throws std::system_error when there is an non-transient error.
   * @returns the number of bytes read.
   *          This will be 0 if there was a transient error
   *          (e.g. `EINTR` or `EAGAIN`).
   */
  template <class C>
  std::size_t try_read(C &buffer) {
    ssize_t bytes =
        read(posix_file_descriptor, std::data(buffer), std::size(buffer));
    if (bytes <= -1) {
      switch (errno) {
        case EINTR:
          [[fallthrough]];
        case EAGAIN:
          return false;
        default: {
          auto exception =
              std::system_error(std::error_code(errno, std::system_category()));
          spdlog::error("Failed to read file at {} due to {}", path.string(),
                        exception.what());
          throw exception;
        }
      }
    }
    return static_cast<std::size_t>(bytes);
  }

  virtual ~PosixFileReadOnly() {
    constexpr int MAX_RETRIES = 128;
    for (int i = 0; i < MAX_RETRIES; i++) {
      if (close(posix_file_descriptor) == 0) {
        return;
      }
      if (errno == EINTR) {
        continue;  // interuptted, retry
      } else {
        auto exception =
            std::system_error(std::error_code(errno, std::system_category()));
        spdlog::error("Failed to close file at {} due to {}", path.string(),
                      exception.what());
        return;
      }
    }
    auto exception =
        std::system_error(std::error_code(EINTR, std::system_category()));
    spdlog::error("Failed to close file at {} after {} iterations due to {}",
                  path.string(), MAX_RETRIES, exception.what());
  }

  int fd() { return posix_file_descriptor; }

 private:
  int posix_file_descriptor = -1;
  std::filesystem::path path;
};

/**
 * Handles reading from the IRLogger log file and calling the given callback
 * function.
 */
class IRLoggerReader {
 public:
  IRLoggerReader(LoggingCallback _logging_callback,
                 const std::filesystem::path &socket_path)
      : ir_logger_parser{_logging_callback} {
    start_thread(socket_path);
  }

  virtual ~IRLoggerReader() {
    try {
      stop_thread();
    } catch (std::exception &e) {
      spdlog::error("IRLoggerReader thread crashed: {}", e.what());
    }
  }

 private:
  void start_thread(const std::filesystem::path &socket_path) {
    spdlog::debug("Making FIFO at {} for logging", socket_path.string());

    if (mkfifo(socket_path.string().c_str(), 0600) != 0) {
      if (errno != EEXIST) {
        auto exception =
            std::system_error(std::error_code(errno, std::system_category()));
        spdlog::error("IRLoggerReader failed to create pipe at {} due to {}",
                      socket_path.string(), exception.what());
        throw exception;
      }
    }

    spdlog::debug("starting thread");
    logger_thread = std::async(std::launch::async, [this, socket_path]() {
      spdlog::debug("Started new thread to read from {}", socket_path.string());

      thread_handle = pthread_self();

      // Ignore SIGUSR1 signals on this thread.
      // POSIX functions will be interrupted and fail with EINTR
      struct sigaction signal_action = {};
      signal_action.sa_handler = SIG_IGN;
      signal_action.sa_flags &=
          ~SA_RESTART;  // force functions to throw EINTR on signals

      if (sigaction(SIGUSR1, &signal_action, nullptr) != 0) {
        auto exception =
            std::system_error(std::error_code(errno, std::system_category()));
        spdlog::error(
            "IRLoggerReader failed to create SIGUSR1 signal handler due to "
            "{}",
            exception.what());
        throw exception;
      }

      auto fifo_pipe = PosixFileReadOnly(socket_path);

      struct pollfd fifo_pollfd = {};
      fifo_pollfd.fd = fifo_pipe.fd();
      fifo_pollfd.events |= POLL_IN;

      auto read_buffer = std::array<char, 1024>();

      while (stop == false) {
        /*
         * Check every 5 seconds if `stop == true`.
         *
         * Normally, poll() should immediately throw an EINTR when a USRSIG1
         * signal is caught, but for reason, this only works some of the
         * time. Once we fix this we, could set the timeout to `-1`
         * (infinity).
         */
        constexpr int POLL_TIMEOUT = 5000;
        int result = poll(&fifo_pollfd, 1, POLL_TIMEOUT);

        if (result == 0) {  // timeout, loop again
          continue;
        } else if (result == -1) {  // handle error
          if (errno == EINTR) {
            // signal interrupt is fine, we just need to check if `stop ==
            // true`.
            continue;
          }
          auto exception =
              std::system_error(std::error_code(errno, std::system_category()));
          spdlog::error("polling FIFO file at {} failed due to {}",
                        socket_path.string(), exception.what());
          throw exception;
        }

        if (fifo_pollfd.revents & POLLERR) {
          // POLL error? TODO: what does this mean?
          spdlog::warn("polling FIFO file at {} got POLLERR",
                       socket_path.string());
        }
        if (fifo_pollfd.revents & (POLLIN | POLLPRI)) {
          // data is available for reading on the pipe
          std::size_t bytes = fifo_pipe.try_read(read_buffer);
          if (bytes > 0) {
            handle_bytes(std::data(read_buffer), bytes);
          }
        }
      }
    });
  }

  void stop_thread() {
    auto no_gil = pybind11::gil_scoped_release();
    stop = true;

    auto thread_handle_value = thread_handle.load();
    if (thread_handle_value) {
      // this should interrupt any slow read() or poll() commands, and make
      // the child thread exit faster (although it only works most of the time)
      pthread_kill(thread_handle_value, SIGUSR1);
    }
    logger_thread.get();
  }

  std::shared_future<void> logger_thread;

  /** If `true`, stop the logger thread */
  std::atomic<bool> stop = false;

  /**
   * Logging thread handle, only used to send signals to the child thread to
   * interrupt it (stops it quicker).
   */
  std::atomic<pthread_t> thread_handle = 0;

  /**
   * Handles parsing the logs.
   */
  IRLoggerParser ir_logger_parser;

  /*
   * Handle some bytes from the log file
   */
  void handle_bytes(const char *bytes, std::size_t size) {
    ir_logger_parser.push_data(bytes, size);
  }
};

/**
 * Gets the real filename that the evo::IRLogger writes to.
 *
 * If you input `my-file.log`, you'll get something like
 * `my-file.log_18_9_2023_21-27-53.log` out.
 *
 * @param irlogger_log_path_prefix - The prefix of the log file.
 * @param delay_if_at_end_of_second - A number of milliseconds between 1000 and
 * 0. If we're in the last bit of a second, block this function from returning
 * until the next second to avoid race-conditions.
 */
static std::filesystem::path irlogger_log_path(
    const std::filesystem::path &irlogger_log_path_prefix,
    std::chrono::milliseconds delay_if_at_end_of_second =
        std::chrono::milliseconds(100)) {
  nqm::irimager::wait_if_at_end_of_second(delay_if_at_end_of_second);

  return irlogger_log_path_prefix.string() + "_" +
         nqm::irimager::evo_irlogger_datestring(
             std::chrono::system_clock::now()) +
         ".log";
}

#ifdef IR_IMAGER_MOCK

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

/**
 * Mock class that logs data to given socket.
 */
class IRLoggerImpl {
 public:
  IRLoggerImpl(const std::filesystem::path &socket_path_prefix) {
    auto real_socket_path = irlogger_log_path(socket_path_prefix);
    irlogger_mock_thread = std::async(
        std::launch::async,
        [real_socket_path,  // by-copy capture, since this will run in another
                            // thread
         this] {
          auto out = std::ofstream(real_socket_path);
          out << "WARNING [irlogger_to_spd_posix.cpp:" << __LINE__
              << "] @ 0.01s :Mocking IRLogger output." << std::endl;
          for (int i = 0; i < 3600 && !this->stop; i++) {
            out << "DEBUG [irlogger_to_spd_posix.cpp:" << __LINE__ << "] @ "
                << i << "s :This is some dummy mocked IRLogger output."
                << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          }
        });
  }

  ~IRLoggerImpl() { stop_thread(); }

 private:
  std::shared_future<void> irlogger_mock_thread;
  std::atomic<bool> stop = false;

  void stop_thread() {
    auto no_gil = pybind11::gil_scoped_release();
    stop = true;
    irlogger_mock_thread.get();
  }
};
#else /* IR_IMAGER_MOCK */

#include <libirimager/IRLogger.h>

/**
 * @brief Run the given function with a timeout.
 *
 * @param function The function to run.
 * @param timeout The timeout.
 * @throws std::system_error if killing the function failed.
 * @throws std::exception from the function if the function throws an exception.
 * @retval std::future_status::ready if the function finished on time.
 * @retval std::future_status::timeout if the function timed-out.
 */
std::future_status run_with_timeout(std::function<void()> function,
                                    std::chrono::milliseconds timeout) {
  auto task = std::packaged_task<void()>(function);
  auto future = task.get_future();

  auto thread = std::thread(std::move(task));

  switch (future.wait_for(timeout)) {
    case std::future_status::ready:
      thread.join();
      future.get();  // throws an exception if the thread threw an exception.
      return std::future_status::ready;  // success!
    case std::future_status::deferred:
      thread.detach();
      // TODO: check if this is impossible
      throw new std::runtime_error("deferred");
    case std::future_status::timeout:
      // thread.native_handle() may only exist on POSIX, not on windows!!
      if (pthread_cancel(thread.native_handle()) != 0) {
        thread.detach();  // will leak, but this should almost never happen
        throw std::system_error(std::error_code(errno, std::system_category()),
                                "Failed to stop thread after timeout");
      }
      thread.join();
      return std::future_status::timeout;
  }

  throw new std::runtime_error("Unknown std::future_status value");
}

/**
 * Logs data to given socket.
 */
class IRLoggerImpl {
 private:
  static constexpr auto TIMEOUT = std::chrono::seconds(5);

 public:
  IRLoggerImpl(const std::filesystem::path &socket_path_prefix) {
    spdlog::debug("Starting evo::IRLogger to file {}.some_time",
                  socket_path_prefix.string());
    auto no_gil = pybind11::gil_scoped_release();  // the setVerbosity()
                                                   // function freezes sometimes

    // evo::IRLogger::setVerbosity() has a bug that occasionally causes it to
    // freeze due to a deadlock, see
    // https://github.com/nqminds/nqm-irimager/issues/51#issuecomment-1750614144
    // Having a timeout is a crappy workaround, but it's better than nothing.
    if (run_with_timeout(std::bind(evo::IRLogger::setVerbosity,
                                   evo::IRLoggerVerbosityLevel::IRLOG_OFF,
                                   evo::IRLoggerVerbosityLevel::IRLOG_DEBUG,
                                   socket_path_prefix.string().c_str()),
                         TIMEOUT) == std::future_status::timeout) {
      throw new std::runtime_error(
          "Timeout when calling evo::IRLogger::setVerbosity");
    }
    spdlog::trace("Started evo::IRLogger");
  }

  ~IRLoggerImpl() {
    if (run_with_timeout(
            std::bind(evo::IRLogger::setVerbosity,
                      evo::IRLoggerVerbosityLevel::IRLOG_OFF,
                      evo::IRLoggerVerbosityLevel::IRLOG_OFF, nullptr),
            TIMEOUT) == std::future_status::timeout) {
      spdlog::error("Timeout when trying to stop evo::IRLogger");
    }
  }
};
#endif /* IR_IMAGER_MOCK */

struct IRLoggerToSpd::impl {
 public:
  impl(const LoggingCallback &logging_callback,
       const std::filesystem::path &socket_path)
      : log_file_reader(logging_callback, irlogger_log_path(socket_path)),
        irlogger_impl(socket_path) {}

 private:
  IRLoggerReader log_file_reader;
  IRLoggerImpl irlogger_impl;
};

/**
 * @see IRLoggerToSpd::IRLoggerToSpd() documentation.
 */
static std::filesystem::path default_socket_path() {
  const char *xdg_runtime_dir_ptr = std::getenv("XDG_RUNTIME_DIR");
  if (xdg_runtime_dir_ptr) {
    auto xdg_runtime_dir =
        std::filesystem::path(xdg_runtime_dir_ptr) / "nqm-irimager";
    try {
      std::filesystem::create_directory(xdg_runtime_dir);
      return xdg_runtime_dir / "irlogger.fifo";
    } catch (const std::filesystem::filesystem_error &e) {
      // on no_such_file_or_directory error, just use temp_directory_path()
      if (e.code() != std::errc::no_such_file_or_directory) {
        throw e;
      }
    }
  }

  auto temp_dir = std::filesystem::temp_directory_path() / "nqm-irimager";
  std::filesystem::create_directory(temp_dir);
  return temp_dir / "irlogger.fifo";
}

IRLoggerToSpd::IRLoggerToSpd(const LoggingCallback &logging_callback)
    : IRLoggerToSpd(logging_callback, default_socket_path()) {}

IRLoggerToSpd::IRLoggerToSpd(const LoggingCallback &logging_callback,
                             const std::filesystem::path &socket_path) {
  auto gil = pybind11::gil_scoped_acquire();
  pImpl = std::make_unique<IRLoggerToSpd::impl>(logging_callback, socket_path);
}

IRLoggerToSpd::~IRLoggerToSpd() {
  auto gil = pybind11::gil_scoped_acquire();
  pImpl = nullptr;
}
