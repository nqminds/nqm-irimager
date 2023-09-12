#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>

#include "../src/nqm/irimager/irlogger_parser.hpp"

// Demonstrate some basic assertions.
TEST(test_irlogger_parser, BasicAssertions) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::debug, "[IRDeviceParams.cpp:147] serial: 0"));
  EXPECT_CALL(mock_callback,
              Call(LogLevel::info,
                   "[IRDeviceCreate.cpp:23] Searching for attached device..."));
  EXPECT_CALL(mock_callback, Call(LogLevel::warn,
                                  "[IRImager.cpp:1089] Will not set focus "
                                  "motor pos --> no focus motor"));
  EXPECT_CALL(mock_callback, Call(LogLevel::error,
                                  "[IRDeviceCreate.cpp:47] No device found!"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());
  parser.push_data("DEBUG [IRDeviceParams.cpp:147] @ 0.00286499s :serial: 0\n");
  parser.push_data(
      "INFO [IRDeviceCreate.cpp:23] @ 0.004448s :Searching for attached "
      "device...\n");
  parser.push_data(
      "WARNING [IRImager.cpp:1089] @ 1.42395s :Will not set focus motor pos "
      "--> no focus motor\n");
  parser.push_data(
      "ERROR [IRDeviceCreate.cpp:47] @ 0.00513013s :No device found!\n");
}

// should handle `push_data()` pushing less than a log-line
TEST(test_irlogger_parser, HandlesPartialLines) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::info, "[test_irlogger_parser.cpp] Hello World!"));
  EXPECT_CALL(mock_callback,
              Call(LogLevel::info,
                   "[test_irlogger_parser.cpp] Hello World numero duo!"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  {
    auto log_string = std::string(
        "INFO [test_irlogger_parser.cpp] @ 0.0000s :Hello World!\n");
    parser.push_data(log_string.substr(0, log_string.size() / 2));
    parser.push_data(log_string.substr(log_string.size() / 2));
  }

  {
    auto log_string = std::string_view(
        "INFO [test_irlogger_parser.cpp] @ 0.12345s :Hello World numero "
        "duo!\n");

    for (std::size_t i = 0; i < log_string.size(); i++) {
      parser.push_data(log_string.substr(i, 1));
    }
  }
}

// should handle multiple lines of logs in a single `push_data()`
TEST(test_irlogger_parser, HandlesMultipleLines) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::info, "[test_irlogger_parser.cpp] Hello World!"));
  EXPECT_CALL(mock_callback,
              Call(LogLevel::info,
                   "[test_irlogger_parser.cpp] Hello World numero duo!"));
  EXPECT_CALL(mock_callback,
              Call(LogLevel::info,
                   "[test_irlogger_parser.cpp] Hello World numero tres!"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  parser.push_data(
      "INFO [test_irlogger_parser.cpp] @ 0.1s :Hello World!\n"
      "INFO [test_irlogger_parser.cpp] @ 0.2s :Hello World numero duo!\n"
      "INFO [test_irlogger_parser.cpp] @ 0.3s :Hello World numero tres!\n");
}

/**
 * Should test that invalid lines are properly logged without an exception.
 */
TEST(test_irlogger_parser, HandlesInvalidLine) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::warn,
                   "Failed to parse IRLogger line due to error: Failed to "
                   "match regex. Line was This line is purposely invalid"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  parser.push_data("This line is purposely invalid\n");
}

/**
 * Should test that invalid lines are properly logged without an exception.
 */
TEST(test_irlogger_parser, HandlesInvalidLineWithBadLogLevel) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::warn,
                   "Failed to parse IRLogger line due to error: Failed to "
                   "parse IRLogger log level value. Line was "
                   "MY_CUSTOM_LOG_LEVEL [test_irlogger_parser.cpp] @ 0.1s "
                   ":This message has a custom log level!"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  parser.push_data(
      "MY_CUSTOM_LOG_LEVEL [test_irlogger_parser.cpp] @ 0.1s :This message has "
      "a custom log level!\n");
}

/**
 * Should test that invalid lines are properly logged without an exception.
 *
 * @todo Should we ignore empty lines?
 */
TEST(test_irlogger_parser, HandlesInvalidEmptyLine) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback, Call(LogLevel::warn,
                                  "Failed to parse IRLogger line due to error: "
                                  "Failed to match regex. Line was "));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  parser.push_data("\n");
}

// Should handle inputs larger than the string buffer
TEST(test_irlogger_parser, HandlesLargeInputs) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  const auto one_log_line =
      std::string("INFO [test_irlogger_parser.cpp] @ 0.1s :Hello World!\n");

  auto my_big_string = std::string();
  my_big_string.reserve(IRLoggerParser::STRING_BUFFER_SIZE +
                        one_log_line.size() * 2);

  int log_lines = 0;

  while (my_big_string.size() <
         IRLoggerParser::STRING_BUFFER_SIZE + one_log_line.size()) {
    my_big_string += one_log_line;
    log_lines++;
  }

  EXPECT_CALL(mock_callback,
              Call(LogLevel::info, "[test_irlogger_parser.cpp] Hello World!"))
      .Times(::testing::Exactly(log_lines));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());
  parser.push_data(my_big_string);
}

// Should handle overflows of the ring buffer
TEST(test_irlogger_parser, HandlesOverflows) {
  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;

  EXPECT_CALL(mock_callback,
              Call(LogLevel::warn,
                   ::testing::StartsWith("IRLoggerParser ring buffer "
                                         "overflow, dumped contents are: "
                                         "INFO [test_irlogger_parser] @ 0s "
                                         ":My super-long string is......."
                                         "abcdefghijklmnopqrstuvwxyzABCDEFGHIJ"
                                         "KLMNOPQRSTUVWXYZ0123456789!?")));

  EXPECT_CALL(mock_callback,
              Call(LogLevel::info, "[test_irlogger_parser.cpp] Hello World!"));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());

  auto prepend = std::string(
      "INFO [test_irlogger_parser] @ 0s :My super-long string is.......");
  parser.push_data(prepend);

  auto chunk = std::string(
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?");

  EXPECT_EQ(prepend.size(),
            chunk.size());  // otherwise test won't work properly
  EXPECT_EQ(IRLoggerParser::STRING_BUFFER_SIZE % chunk.size(), 0);
  for (std::size_t i = prepend.size(); i < IRLoggerParser::STRING_BUFFER_SIZE;
       i += chunk.size()) {
    parser.push_data(chunk);
  }

  parser.push_data("INFO [test_irlogger_parser.cpp] @ 0.0000s :Hello World!\n");
}

class TestIRLoggerParserWithLogFile
    : public testing::TestWithParam<std::filesystem::path> {};

// load a log file into memory, and see if running `push_data()` works with it
TEST_P(TestIRLoggerParserWithLogFile, WorksOnLogFileAllAtOnce) {
  auto path = GetParam();

  auto input_stream = std::ifstream(path, std::ios::in);

  auto str = std::string(std::istreambuf_iterator<char>{input_stream}, {});

  int log_lines = 0;
  for (std::size_t pos = 0; pos < str.length(); pos++) {
    log_lines += (str[pos] == '\n');
  }

  testing::MockFunction<void(LogLevel, std::string_view)> mock_callback;
  EXPECT_CALL(mock_callback, Call(::testing::_, ::testing::_))
      .Times(::testing::Exactly(log_lines));

  auto parser = IRLoggerParser(mock_callback.AsStdFunction());
  parser.push_data(str);
}

INSTANTIATE_TEST_SUITE_P(
    RealLogs, TestIRLoggerParserWithLogFile,
    ::testing::Values((std::filesystem::path(__FILE__).parent_path() /
                       "__fixtures__" / "piimager.error.log"),
                      (std::filesystem::path(__FILE__).parent_path() /
                       "__fixtures__" / "piimager.regular.log")));
