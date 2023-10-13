#include <gtest/gtest.h>

#include <filesystem>

#include "../src/nqm/irimager/irimager_class.hpp"

static std::filesystem::path XML_FILE;

// Demonstrate some basic assertions.
TEST(test_get_temp_range_decimal, BasicAssertions) {
  auto irimager =
      IRImagerMock(XML_FILE.string().data(), XML_FILE.string().size());
  EXPECT_GE(irimager.get_temp_range_decimal(),
            0);  // this value should almost always be 1
}

/**
 * Should throw an error when trying to open a non-existent XML file.
 */
TEST(test_irimager_class, NonExistentFile) {
  auto path = std::filesystem::path("this-file-should-not-exist");
  EXPECT_THROW(IRImagerMock(path.string().data(), path.string().size()),
               std::runtime_error);
}

int main(int argc, char **argv) {
  XML_FILE = std::filesystem::path(argv[0]).parent_path() / "__fixtures__" /
             "382x288@27Hz.xml";

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
