#include "./logger.hpp"

#include <atomic>

#include <spdlog/sinks/callback_sink.h>
#include <spdlog/spdlog.h>

#include "./irlogger_to_spd.hpp"

/** Map spdlog's spd::level::level_enum enum to our LogLevel enum */
static LogLevel spd_level_to_irimager_level(
    spdlog::level::level_enum input) noexcept {
  switch (input) {
    case spdlog::level::level_enum::warn:
      return LogLevel::warn;
    case spdlog::level::level_enum::debug:
      return LogLevel::debug;
    case spdlog::level::level_enum::info:
      return LogLevel::info;
    case spdlog::level::level_enum::err:
      return LogLevel::error;
    case spdlog::level::level_enum::critical:
      return LogLevel::critical;
    case spdlog::level::level_enum::off:
    case spdlog::level::level_enum::trace:
    case spdlog::level::level_enum::n_levels:
      return LogLevel::trace;
  }

  // this should never happen, but if it does, we shouldn't throw an exception
  return LogLevel::trace;
}

struct Logger::impl {
 public:
  inline static std::weak_ptr<Logger::impl> singleton;
  inline static std::mutex singleton_mutex;

  /**
   * @brief Construct a new impl object, redirecting logs to `logging_callback`.
   *
   * This function is not thread-safe, and should be protected by
   * singleton_mutex.
   *
   * @param logging_callback The function to call with log data.
   */
  impl(LoggingCallback logging_callback) {
    redirect_spd(logging_callback);

    spdlog::debug("set up Python logging callback");

    // construct after calling redirect_spd, so we can see logs during
    // construction
    ir_logger_to_spd = std::make_unique<IRLoggerToSpd>(logging_callback);
  }

  virtual ~impl() {
    ir_logger_to_spd = nullptr;
    reset_spd_redirect();
  }

 private:
  /** If we've called redirect_spd(), this var stores the original logger */
  std::shared_ptr<spdlog::logger> old_logger;

  /** Handles piping IRImageSDK logs to spdlog */
  std::unique_ptr<IRLoggerToSpd> ir_logger_to_spd;

  /** Redirects calls to `spdlog::log()` in C++ to the given callback */
  void redirect_spd(LoggingCallback logging_callback) {
    reset_spd_redirect();

    auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_st>(
        [logging_callback](const spdlog::details::log_msg &msg) {
          logging_callback(
              spd_level_to_irimager_level(msg.level),
              // msg.payload might be a fmt::string_view, not a
              // std::string_view, so we might need to convert it. This won't be
              // needed once we are using C++20, since then fmtlib is not used.
              std::string_view(msg.payload.data(), msg.payload.size()));
        });

    // pass all logs to the callback
    callback_sink->set_level(spdlog::level::trace);

    auto callback_logger =
        std::make_shared<spdlog::logger>("callback_sink", callback_sink);

    old_logger = spdlog::default_logger();
    spdlog::set_default_logger(callback_logger);

    // pass all spdlogs to the callback
    spdlog::set_level(spdlog::level::trace);

    spdlog::debug("Redirecting spdlogs to a callback.");
  }

  void reset_spd_redirect() {
    if (!old_logger) {
      return;
    }
    spdlog::set_default_logger(old_logger);
    old_logger = nullptr;
  }
};

static LoggingCallback log_to_python(pybind11::object logger) {
  return [logger](LogLevel log_level, const std::string_view msg) {
    auto gil = pybind11::gil_scoped_acquire();
    logger.attr("log")(static_cast<int>(log_level), "%s", msg);
  };
}

static pybind11::object default_logger() {
  auto gil = pybind11::gil_scoped_acquire();
  return pybind11::module_::import("logging").attr("getLogger")("nqm.irimager");
}

Logger::Logger(LoggingCallback logging_callback) {
  auto no_gil =
      pybind11::gil_scoped_release();  // release gil to avoid deadlock
  auto singleton_lock = std::scoped_lock(Logger::impl::singleton_mutex);
  if (Logger::impl::singleton.use_count()) {
    throw std::runtime_error(
        "Only a single instance of the Logger should be active at the same "
        "time.");
  }

  {
    auto gil = pybind11::gil_scoped_acquire();
    pImpl = std::make_shared<Logger::impl>(logging_callback);
    Logger::impl::singleton = pImpl;
  }
}

Logger::Logger(pybind11::object logger)
    : Logger::Logger(log_to_python(logger)) {}

Logger::Logger() : Logger::Logger(default_logger()) {}

Logger::~Logger() {
  // release Python GIL, to avoid deadlocks
  auto no_gil = pybind11::gil_scoped_release();
  auto singleton_lock = std::scoped_lock(Logger::impl::singleton_mutex);

  {
    auto gil = pybind11::gil_scoped_acquire();
    pImpl = nullptr;
  }
}
