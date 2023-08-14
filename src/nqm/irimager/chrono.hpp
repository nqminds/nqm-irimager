#ifndef CHRONO_HPP
#define CHRONO_HPP

#include <chrono>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/chrono.h>  // needed for logging/formatting std::chrono

namespace nqm {
namespace irimager {

/**
 * Converts from `steady_clock` to `system_clock`.
 *
 * Converts a time_point from std::chrono::steady_clock (time since last boot)
 * to std::chrono::system_clock (aka time since UNIX epoch).
 *
 * C++20 has a function called std::chrono::clock_cast that will do this
 * for us, but we're stuck on C++17, so instead we have to do this imprecise
 * monstrosity to do the conversion.
 *
 * @remarks
 * This function is imprecise!!! Calling it multiple times with the same data
 * will result in different results.
 */
inline std::chrono::time_point<std::chrono::system_clock> clock_cast(
    const std::chrono::time_point<std::chrono::steady_clock>
        &steady_time_point) {
  auto sys_now = std::chrono::system_clock::now();
  auto sdy_now = std::chrono::steady_clock::now();
  return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      steady_time_point - sdy_now + sys_now);
}

/**
 * Finds the next whole second.
 */
inline std::chrono::time_point<std::chrono::system_clock> next_second(
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now()) {
  auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

  return now_seconds + std::chrono::seconds(1);
}

/**
 * Block/wait until the next second if we're in the last @p period of a second.
 */
template <typename Rep, typename Period>
inline void wait_if_at_end_of_second(
    std::chrono::duration<Rep, Period> period) {
  auto now = std::chrono::system_clock::now();

  if (next_second(now) - now < period) {
    spdlog::debug("Waiting {} until next second", next_second(now) - now);
    std::this_thread::sleep_until(next_second(now));
    spdlog::trace("Waited {}", std::chrono::system_clock::now() - now);
  }
}

/**
 * Gets the datestring format that evo::IRLogger uses.
 *
 * E.g., for time `2023-09-18T21:27:53`, returns `18_9_2023_21-27-53`.
 *
 * Please be aware, that due to daylight savings or other timezone changes,
 * multiple times may map to the same datestring 😭.
 *
 * All of these numbers don't use 0 prefixes, so we can't use
 * std::formatter<std::chrono::sys_time>.
 */
inline std::string evo_irlogger_datestring(
    const std::chrono::time_point<std::chrono::system_clock> &time_point) {
  auto t = std::chrono::system_clock::to_time_t(time_point);
  auto tm_ptr = std::localtime(&t);
  if (tm_ptr == nullptr) {
    // remind me: this code will overflow in the year 2 147 483 648 AD
    throw std::system_error(errno, std::generic_category());
  }
  auto tm = *tm_ptr;

  auto stringstream = std::stringstream();

  // all of these numbers don't have leading `0`s 😭😭😭
  stringstream << tm.tm_mday << "_" << tm.tm_mon + 1 << "_" << tm.tm_year + 1900
               << "_" << tm.tm_hour << "-" << tm.tm_min << "-" << tm.tm_sec;

  return stringstream.str();
}

}  // namespace irimager
}  // namespace nqm

#endif /* CHRONO_HPP */
