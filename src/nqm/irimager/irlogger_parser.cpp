#include "./irlogger_parser.hpp"

#include <regex>
#include <unordered_map>

static const std::unordered_map<std::string, LogLevel>
    parse_ir_logger_log_level = {{"DEBUG", LogLevel::debug},
                                 {"INFO", LogLevel::info},
                                 {"WARNING", LogLevel::warn},
                                 {"ERROR", LogLevel::error}};

/**
 * @brief Parse the given IRLogger log line.
 *
 * For an example:
 *
 * ```c++
 * // should return {LogLevel::error, "[IRDeviceCreate.cpp:47] No device
 * //                found!"};
 * parse_line("ERROR [IRDeviceCreate.cpp:47] @ 0.00513013s :No device found!");
 * ```
 *
 * @throws std::logic_error if parsing the IRLogger log line failed.
 */
static std::tuple<LogLevel, std::string> parse_line(const std::string &line) {
  auto regex =
      std::regex(R"""((\w+)\ \[([\w.:\d]*)\]\ \@\ ([\d\.]+s)\ :(.*))""",
                 std::regex_constants::ECMAScript);

  std::smatch match_results;

  if (!std::regex_match(line, match_results, regex)) {
    throw std::logic_error("Failed to match regex.");
  }

  const auto log_level = match_results[1];
  const auto location = match_results[2];
  const auto message = match_results[4];

  try {
    return {
        parse_ir_logger_log_level.at(log_level),
        "[" + location.str() + "] " + message.str(),
    };
  } catch (std::out_of_range &e) {
    throw std::logic_error("Failed to parse IRLogger log level value.");
  }
}

void IRLoggerParser::push_data(const char *data, std::size_t length) {
  auto string_view = std::string_view(data, length);
  push_data(string_view);
}

void IRLoggerParser::push_data(std::string_view data) {
  if (buffer.size() + data.size() <= STRING_BUFFER_SIZE) {
    buffer.insert(data);

    while (try_parse())
      ;
  } else {
    if (STRING_BUFFER_SIZE == buffer.size()) {
      // buffer is full, log and dump buffer
      logging_callback(
          LogLevel::warn,
          "IRLoggerParser ring buffer overflow, dumped contents are: " +
              buffer.peek());
      buffer.discard(buffer.size());  // empty buffer
    }

    auto data_to_insert_now =
        data.substr(0, STRING_BUFFER_SIZE - buffer.size());

    // try pushing only a subset of the data
    buffer.insert(data_to_insert_now);

    while (try_parse())
      ;

    // since this is the last-line of the function, tail-call optimization
    // should prevent a stack overflow
    push_data(data.substr(data_to_insert_now.size()));
  }
}

bool IRLoggerParser::try_parse() {
  auto buffer_contents = buffer.peek();

  auto index = buffer_contents.find("\n");

  if (index == std::string::npos) {
    return false;
  }

  // line not including `\n` character
  auto line = buffer_contents.substr(0, index);
  buffer.discard(index + 1);  // discard line **AND** `\n` character

  try {
    auto [log_level, message] = parse_line(line);
    logging_callback(log_level, message);
  } catch (std::exception &e) {
    // prevent exceptions from crashing the program.
    logging_callback(
        LogLevel::warn,
        std::string("Failed to parse IRLogger line due to error: ") + e.what() +
            " Line was " + line);
  }

  return true;
}
