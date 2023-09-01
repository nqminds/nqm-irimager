#include <gtest/gtest.h>

#include <iomanip>
#include <sstream>
#include <utility>

#include "../src/nqm/irimager/chrono.hpp"

std::chrono::time_point<std::chrono::system_clock> parse_iso8601(
    std::string iso8601_datetime) {
  auto stringstream = std::istringstream();
  stringstream.str(iso8601_datetime);
  stringstream.exceptions(std::ios_base::failbit);

  std::tm tm = {};
  stringstream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

  // daylight savings usually only occurs between 23:00 and 03:59, so if we
  // ignore these hours, we shouldn't have to worry about ambigious dates
  // (we're stuck on C++17, so we can't use stdlibc++ timezone funcs)
  if (23 <= tm.tm_hour || tm.tm_hour < 4) {
    throw new std::invalid_argument("The date " + iso8601_datetime +
                                    " is potentially ambigious due to daylight "
                                    "savings. Please pick a different hour.");
  }
  tm.tm_isdst = -1;  // pick DST/not-DST automatically

  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

class TestEvoIRLoggerDatestring
    : public testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(TestEvoIRLoggerDatestring, ShouldMatchExpectedValue) {
  const auto& [iso_date, expected_output] = GetParam();

  EXPECT_EQ(nqm::irimager::evo_irlogger_datestring(parse_iso8601(iso_date)),
            expected_output);
}

INSTANTIATE_TEST_SUITE_P(ShouldMatchExpectedValue, TestEvoIRLoggerDatestring,
                         ::testing::Values((std::pair<std::string, std::string>{
                             "2023-09-18T21:27:53", "18_9_2023_21-27-53"})));
