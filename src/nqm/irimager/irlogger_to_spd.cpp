#include "./irlogger_to_spd.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <exception>
#include <future>
#include <memory>
#include <uvw.hpp>

class IRLoggerToSpd::impl {
 public:
  impl(const std::filesystem::path &socket_path) { start_thread(socket_path); }

  virtual ~impl() {
    try {
      stop_thread();
    } catch (std::exception &e) {
      spdlog::error("IRLoggerToSpd thread crashed: {}", e.what());
    }
  }

 private:
  void start_thread(const std::filesystem::path &socket_path) {
    spdlog::debug("starting thread");

    logger_thread = std::async(
        std::launch::async,
        [this,
         socket_path  // capture by value, since this will run in another thread
    ] {
          spdlog::debug("started in new thread");

          auto loop = uvw::loop::get_default();

          // TODO: do we need to check for nullptr?
          this->message = loop->resource<uvw::async_handle>();

          spdlog::debug("checking for early stop");

          if (this->stop) {  // in case calling thread stopped this before
                             // `message` was created
            spdlog::info("early exit!");
            loop->stop();
          }

          this->message->on<uvw::async_event>(
              [this, &loop](const uvw::async_event &, uvw::async_handle &) {
                if (this->stop) {
                  loop->stop();
                }
              });

          spdlog::debug("message handler added");

#ifdef FIFO_STREAMING_WORKKING
          auto log_file_reader = loop->resource<uvw::pipe_handle>();

          log_file_reader->on<uvw::connect_event>(
              [](const uvw::connect_event &, uvw::pipe_handle &pipe_handle) {
                spdlog::debug("Connection");
                pipe_handle.on<uvw::end_event>(
                    [](const uvw::end_event &, uvw::pipe_handle &srv) {
                      srv.close();
                    });
                pipe_handle.on<uvw::data_event>(
                    [](const uvw::data_event &data, uvw::pipe_handle &) {
                      auto string_data =
                          std::string_view(data.data.get(), data.length);

                      // TODO PARSE LOGS
                      spdlog::info(string_data);
                    });
                pipe_handle.read();
              });

          log_file_reader->on<uvw::error_event>(
              [&loop](const uvw::error_event &error, uvw::pipe_handle &) {
                spdlog::error("log_file_reader error {}", error.what());
              });

          spdlog::debug("Connecting to socket {}", socket_path.string());
          log_file_reader->connect(socket_path.string());
          log_file_reader->read();
#endif

#define UVW_FILE_STREAMING 1
#ifdef UVW_FILE_STREAMING
          /**
           * This currently works, but it's very inefficient, since it maxes
           * out the CPU to 100%!
           */
          auto log_file_reader = loop->resource<uvw::file_req>();

          size_t file_offset = 0;
          log_file_reader->on<uvw::fs_event>([&](const auto &event, auto &req) {
            switch (event.type) {
              case uvw::fs_req::fs_type::READ: {
                if (event.result != 0) {
                  auto string_data =
                      std::string_view(event.read.data.get(), event.result);
                  file_offset += event.result;

                  // TODO: PARSE LOGS somehow?
                  spdlog::info(string_data);
                }

                [[fallthrough]];
              }
              case uvw::fs_req::fs_type::OPEN:
                req.read(static_cast<int64_t>(file_offset), 16);
                break;
              default:
                spdlog::error(
                    "Unexpected fs_req type {}",
                    static_cast<std::underlying_type_t<uvw::fs_req::fs_type>>(
                        event.type));
                [[fallthrough]];
              case uvw::fs_req::fs_type::CLOSE:
                spdlog::debug("Closing file {}", socket_path.string());
                log_file_reader->close();
            }
          });

          spdlog::debug("Reading from file at {}", socket_path.string());
          log_file_reader->open(socket_path.string(),
                                uvw::file_req::file_open_flags::RDONLY, 0644);
#endif  // POSIX hack

          spdlog::debug("running loop in another thread");
          loop->run();
        });
  }

  void stop_thread() {
    stop = true;
    if (message != nullptr) {
      message->send();
    }

    logger_thread.get();
  }

  std::shared_future<void> logger_thread;

  /** Send messages to libuv thread from here */
  std::shared_ptr<uvw::async_handle> message;
  /** If `true`, stop the logger thread when a @p message is received */
  std::atomic<bool> stop = false;
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

IRLoggerToSpd::IRLoggerToSpd() : IRLoggerToSpd(default_socket_path()) {}

IRLoggerToSpd::IRLoggerToSpd(const std::filesystem::path &socket_path)
    : pImpl{std::make_unique<IRLoggerToSpd::impl>(socket_path)} {}

IRLoggerToSpd::~IRLoggerToSpd() {}
