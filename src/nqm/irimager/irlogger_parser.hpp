#ifndef NQM_IRIMAGER_IRLOGGER_PARSER
#define NQM_IRIMAGER_IRLOGGER_PARSER

#include <string_view>

#include "./definitions.hpp"
#include "./string_ring_buffer.hpp"

/**
 * @brief Handle parsing the log output from the evo::IRLogger.
 *
 * @warning This class is not thread-safe.
 */
class IRLoggerParser {
 public:
  IRLoggerParser(LoggingCallback _logging_callback)
      : logging_callback{_logging_callback} {}

  /**
   * Push some data into the IRLoggerParser, automatically calling the
   * callback if a valid log line is found.
   */
  void push_data(std::string_view data);

  /**
   * Wrapper around void push_data(std::string_view data).
   *
   * Due to C++11 ABI issues with evo::IRImager, using std::string_view
   * directly may not be possible.
   */
  void push_data(const char* data, std::size_t data_length);

  static constexpr std::size_t STRING_BUFFER_SIZE = 1 << 20;  // 1 MiB

 private:
  LoggingCallback logging_callback;

  StringRingBuffer<STRING_BUFFER_SIZE> buffer;

  /**
   * @brief Tries to parse a log line from the internal buffer.
   *
   * @return true if a log line was parsed
   * @return false if not log line was found (and the buffer is unchanged)
   */
  bool try_parse();
};

#endif /* NQM_IRIMAGER_IRLOGGER_PARSER */
