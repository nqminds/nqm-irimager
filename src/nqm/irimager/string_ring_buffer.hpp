/**
 * @file
 * @author Alois Klink <alois@nquiringminds.com>
 * @brief Ring buffer designed for string data.
 *
 * @copyright
 * SPDX-FileCopyrightText: © 2023 NquiringMinds Ltd.
 */

#ifndef NQM_IRIMAGER_STRING_RING_BUFFER
#define NQM_IRIMAGER_STRING_RING_BUFFER

#include <array>
#include <stdexcept>
#include <string>
#include <string_view>

/**
 * @brief FIFO Ring buffer for string data.
 *
 * @warning This class is not thread safe.
 *
 * @tparam N The size of the string buffer (allocated on the stack).
 *           For performance reasons, please pick a power of 2 (2ⁿ).
 */
template <std::size_t N>
class StringRingBuffer {
 public:
  /**
   * Returns the number of characters in the buffer.
   */
  size_t size() { return _size; }

  /**
   * @brief Insert the string into the buffer.
   *
   * @throws std::out_of_range if there isn't enough space in the buffer
   *                           for the given string.
   */
  void insert(std::string_view string) {
    if (_size == data.size()) {
      if (!string.empty()) {
        throw std::out_of_range("Not enough space in this ring buffer");
      }
    } else if (begin <= end()) {
      std::size_t space_before_overflow =
          static_cast<std::size_t>(data.size() - end());
      std::size_t bytes_to_write_to_r =
          std::min(string.size(), space_before_overflow);
      string.copy(&data.data()[end()], bytes_to_write_to_r);
      _size += bytes_to_write_to_r;
      if (bytes_to_write_to_r != string.size()) {
        insert(string.substr(bytes_to_write_to_r));
      }
    } else {
      std::size_t space_before_overflow =
          static_cast<std::size_t>(begin - end());
      if (string.size() > space_before_overflow) {
        throw std::out_of_range("Not enough space in this ring buffer");
      }
      string.copy(&data.data()[end()], string.size());
      _size += string.size();
    }
  }

  /**
   * @brief Get the current string in the buffer.
   */
  std::string peek() {
    if (_size == 0) {
      return "";
    }
    if (begin < end()) {
      return std::string(&data.data()[begin],
                         static_cast<std::size_t>(end() - begin));
    } else {
      return std::string(&data.data()[begin],
                         static_cast<std::size_t>(data.size() - begin)) +
             std::string(&data.data()[0], end());
    }
  }

  /**
   * @brief Discard the given number of bytes.
   *
   * @throws std::out_of_range if discarding more bytes than are in the
   *                           buffer.
   */
  void discard(std::size_t bytes) {
    if (bytes > size()) {
      throw std::out_of_range("Not enough bytes in this ring buffer");
    }

    if (begin <= end()) {
      begin += bytes;
      _size -= bytes;

      if (begin >= data.size()) {
        begin = 0;
      }
    } else {
      std::size_t space_before_overflow =
          static_cast<std::size_t>(data.size() - begin);
      std::size_t bytes_to_discard_now = std::min(bytes, space_before_overflow);
      _size -= bytes_to_discard_now;

      if (bytes_to_discard_now != bytes) {
        begin = 0;
        discard(bytes - bytes_to_discard_now);
      } else {
        begin += bytes_to_discard_now;
      }
    }
  }

 private:
  /**
   * @brief Index of the first filled data point.
   *
   * @warning if @p _size is `0`, then this index has no data and means
   *          nothing.
   */
  std::size_t begin = 0;

  /**
   * @brief The number characters stored in this ring buffer.
   */
  std::size_t _size = 0;

  std::array<char, N> data;

  /**
   * @brief Index of the first unfilled data point.
   */
  std::size_t end() { return (begin + _size) % data.size(); }
};

#endif /* NQM_IRIMAGER_STRING_RING_BUFFER */
