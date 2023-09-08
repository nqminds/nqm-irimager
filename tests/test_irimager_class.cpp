#include <gtest/gtest.h>

#include <filesystem>

#include "../src/nqm/irimager/irimager_class.hpp"

static std::filesystem::path XML_FILE;

// Demonstrate some basic assertions.
TEST(test_get_temp_range_decimal, BasicAssertions) {
  auto irimager = IRImager(XML_FILE.string().data(), XML_FILE.string().size());
  EXPECT_GE(irimager.get_temp_range_decimal(),
            0);  // this value should almost always be 1
}

int main(int argc, char **argv) {
  XML_FILE = std::filesystem::path(argv[0]).parent_path() / "__fixtures__" /
             "382x288@27Hz.xml";

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
