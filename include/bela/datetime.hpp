// DATETIME
#ifndef BELA_DATETIME_HPP
#define BELA_DATETIME_HPP
#include "time.hpp"

namespace bela {

enum Month : std::int_least8_t {
  January = 1, // start as
  February,
  March,
  April,
  May,
  June,
  July,
  August,
  September,
  October,
  November,
  December,
};

enum Weekday : std::int_least32_t {
  Sunday = 0, // Sunday
  Monday,
  Tuesday,
  Wednesday,
  Thursday,
  Friday,
  Saturday,
};

class DateTime {
public:
  constexpr explicit DateTime(std::int_least64_t y, std::int_least8_t mon, std::int_least8_t d, std::int_least8_t h,
                              std::int_least8_t m, std::int_least8_t s) noexcept
      : year(y), month(static_cast<bela::Month>(mon)), day(d), hour(h), minute(m), second(s) {}
  DateTime() = default;
  DateTime(bela::Time t);
  constexpr auto Year() const noexcept { return year; }
  constexpr auto Month() const noexcept { return month; }
  constexpr auto Day() const noexcept { return day; }
  constexpr auto Hour() const noexcept { return hour; }
  constexpr auto Minute() const noexcept { return minute; }
  constexpr auto Second() const noexcept { return second; }
  constexpr auto Weekday() const noexcept { return wday; }
  constexpr auto TZDiff() const noexcept { return tzdiff; }
  std::wstring Format(bool full = false); // RFC3339
private:
  std::int_fast64_t year;
  bela::Month month;
  std::int_least8_t day;
  std::int_least8_t hour;
  std::int_least8_t minute;
  std::int_least8_t second;
  bela::Weekday wday;
  std::int_least32_t ms{0};
  std::int_least32_t tzdiff{0}; // second
};

} // namespace bela

#endif