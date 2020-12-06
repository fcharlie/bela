// DATETIME
#ifndef BELA_DATETIME_HPP
#define BELA_DATETIME_HPP
#include "time.hpp"

namespace bela {

struct time_parts {
  int64_t sec;
  uint32_t nsec;
};

inline constexpr time_parts Split(bela::Time t) {
  const auto d = time_internal::ToUnixDuration(t);
  const int64_t rep_hi = time_internal::GetRepHi(d);
  const uint32_t rep_lo = time_internal::GetRepLo(d);
  return {rep_hi, static_cast<uint32_t>(rep_lo / time_internal::kTicksPerNanosecond)};
}

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

inline constexpr bool IsLeapYear(std::int_least64_t y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }
class DateTime;
namespace time_internal {
bool MakeDateTime(int64_t second, DateTime &dt);
};

class DateTime {
public:
  constexpr explicit DateTime(std::int_least64_t y, std::int_least8_t mon, std::int_least8_t d, std::int_least8_t h,
                              std::int_least8_t m, std::int_least8_t s, std::int_least32_t tz = 0) noexcept
      : year(y), month(static_cast<bela::Month>(mon)), day(d), hour(h), minute(m), second(s), tzoffset(tz) {}
  DateTime() = default;
  DateTime(bela::Time t, std::int_least32_t tz = 0) noexcept;
  constexpr auto Year() const noexcept { return year; }
  constexpr auto Month() const noexcept { return month; }
  constexpr int Day() const noexcept { return static_cast<int>(day); }
  constexpr int Hour() const noexcept { return static_cast<int>(hour); }
  constexpr int Minute() const noexcept { return static_cast<int>(minute); }
  constexpr int Second() const noexcept { return static_cast<int>(second); }
  constexpr auto TimeZoneOffset() const noexcept { return tzoffset; }
  auto &TimeZoneOffset() noexcept { return tzoffset; }
  // RFC3339
  std::wstring Format(bool nano = false);
  bela::Time Time() const noexcept;

private:
  friend bool time_internal::MakeDateTime(int64_t second, DateTime &dt);
  std::int_fast64_t year;
  bela::Month month;
  std::int_least8_t day;
  std::int_least8_t hour;
  std::int_least8_t minute;
  std::int_least8_t second;
  std::int_least32_t nsec{0};
  std::int_least32_t tzoffset{0}; // timezone diff
};

std::int_least32_t TimeZoneOffset();

inline bela::DateTime LocalDateTime(bela::Time t) {
  auto tzoffset = TimeZoneOffset();
  return bela::DateTime(t - bela::Seconds(tzoffset), tzoffset);
}

inline std::wstring FormatTime(bela::Time t, bool nano = false) {
  auto dt = LocalDateTime(t);
  return dt.Format(nano);
}
inline std::wstring FormatUniversalTime(bela::Time t, bool nano = false) { return DateTime(t).Format(nano); }

inline constexpr Weekday GetWeekday(const bela::DateTime &dt) noexcept {
  constexpr Weekday k_weekday_by_mon_off[13] = {
      Weekday::Monday,    Weekday::Tuesday,  Weekday::Wednesday, //
      Weekday::Thursday,  Weekday::Friday,   Weekday::Saturday,  //
      Weekday::Sunday,    Weekday::Monday,   Weekday::Tuesday,   //
      Weekday::Wednesday, Weekday::Thursday, Weekday::Friday,    //
      Weekday::Saturday,
  };
  constexpr int k_weekday_offsets[1 + 12] = {
      -1, 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4,
  };
  std::int_least64_t wd = 2400 + (dt.Year() % 400) - (dt.Month() < 3) ? 1 : 0;
  wd += wd / 4 - wd / 100 + wd / 400;
  wd += k_weekday_offsets[dt.Month()] + dt.Day();
  return k_weekday_by_mon_off[wd % 7 + 6];
}

std::wstring_view WeekdayName(Weekday wd, bool shortname = true) noexcept;
std::wstring_view MonthName(Month mon, bool shortname = true) noexcept;
} // namespace bela

#endif