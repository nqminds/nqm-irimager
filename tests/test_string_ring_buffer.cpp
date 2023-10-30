#include <gtest/gtest.h>

#include "../src/nqm/irimager/string_ring_buffer.hpp"

// Demonstrate some basic assertions.
TEST(test_string_ring_buffer, BasicAssertions) {
  auto abc = StringRingBuffer<16>();
  EXPECT_EQ(abc.size(), 0);

  EXPECT_EQ(abc.peek(), std::string(""));

  EXPECT_THROW(abc.discard(1), std::out_of_range);

  abc.insert("a");
  EXPECT_EQ(abc.size(), 1);
  EXPECT_EQ(abc.peek(), std::string("a"));

  abc.discard(1);
  EXPECT_EQ(abc.size(), 0);
  EXPECT_EQ(abc.peek(), std::string(""));

  abc.insert("0123456789abcdef");
  EXPECT_EQ(abc.size(), 16);
  EXPECT_EQ(abc.peek(), std::string("0123456789abcdef"));

  abc.discard(1);
  EXPECT_EQ(abc.size(), 15);
  EXPECT_EQ(abc.peek(), std::string("123456789abcdef"));

  abc.insert("0");
  EXPECT_EQ(abc.size(), 16);
  EXPECT_EQ(abc.peek(), std::string("123456789abcdef0"));

  EXPECT_THROW(abc.insert("1"), std::out_of_range);

  abc.discard(8);
  abc.insert("ABCDEFGH");
  EXPECT_EQ(abc.size(), 16);
  EXPECT_EQ(abc.peek(), std::string("9abcdef0ABCDEFGH"));
}

// should handle NUL (␀) characters
TEST(test_string_ring_buffer, HandlesNulChars) {
  using namespace std::string_literals;

  auto abc = StringRingBuffer<16>();

  // we're not in C-world anymore ␀🧙
  abc.insert("\0\0\0"s);

  EXPECT_EQ(abc.size(), 3);
  EXPECT_EQ(abc.peek(), std::string("\0\0\0"s));
}

// should handle multi-byte characters
TEST(test_string_ring_buffer, HandlesMultibytesChars) {
  auto abc = StringRingBuffer<8>();

  abc.insert("🂢");
  EXPECT_EQ(abc.size(), 4);
  EXPECT_EQ(abc.peek(), "🂢");

  abc.insert("abc");
  EXPECT_EQ(abc.size(), 7);

  abc.discard(4);
  abc.insert("🂢");
  EXPECT_EQ(abc.size(), 7);
  // multi-byte character should look fine, even if it straddles the
  // beginning/end of the circular buffer
  EXPECT_EQ(abc.peek(), "abc🂢");

  abc.discard(4);
  EXPECT_EQ(abc.size(), 3);
  // should show the last three UTF-8 bytes for 🂢
  EXPECT_EQ(abc.peek(), "\x9f\x82\xa2");
}

// should work, even with a size that isn't a power of 2
TEST(test_string_ring_buffer, SupportsAnySize) {
  auto abc = StringRingBuffer<3>();

  abc.insert("123");
  EXPECT_EQ(abc.size(), 3);
  EXPECT_EQ(abc.peek(), std::string("123"));

  abc.discard(1);
  EXPECT_EQ(abc.size(), 2);
  EXPECT_EQ(abc.peek(), std::string("23"));

  abc.insert("4");
  EXPECT_EQ(abc.size(), 3);
  EXPECT_EQ(abc.peek(), std::string("234"));

  EXPECT_THROW(abc.insert("5"), std::out_of_range);

  EXPECT_THROW(abc.discard(4), std::out_of_range);
}
