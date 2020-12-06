///
#include <bela/datetime.hpp>
#include <bela/fmt.hpp>

namespace bela {

constexpr int64_t secondsPerMinute = 60;
constexpr int64_t secondsPerHour = 60 * secondsPerMinute;
constexpr int64_t secondsPerDay = 24 * secondsPerHour;
constexpr int64_t secondsPerWeek = 7 * secondsPerDay;
constexpr int64_t daysPer400Years = 365 * 400 + 97;
constexpr int64_t daysPer100Years = 365 * 100 + 24;
constexpr int64_t daysPer4Years = 365 * 4 + 1;

// The unsigned zero year for internal calculations.
// Must be 1 mod 400, and times before it will not compute correctly,
// but otherwise can be changed at will.
constexpr int64_t absoluteZeroYear = -292277022399;
// The year of the zero Time.
// Assumed by the unixToInternal computation below.
constexpr int64_t internalYear = 1;
constexpr int64_t unixEpochDays = 1969 * 365 + 1969 / 4 - 1969 / 100 + 1969 / 400;
constexpr int64_t unixToInternal = unixEpochDays * secondsPerDay;
constexpr int64_t internalToUnix = -unixToInternal;
constexpr int64_t wallToInternal = (1884 * 365 + 1884 / 4 - 1884 / 100 + 1884 / 400) * secondsPerDay;
constexpr int64_t internalToWall = -wallToInternal;
constexpr int LeapMonths[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
constexpr int Months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

constexpr const wchar_t *longDayNames[] = {
    L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday",
};

constexpr const wchar_t *shortDayNames[] = {
    L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat",
};

constexpr const wchar_t *shortMonthNames[] = {
    L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec",
};

constexpr const wchar_t *longMonthNames[] = {
    L"January", L"February", L"March",     L"April",   L"May",      L"June",
    L"July",    L"August",   L"September", L"October", L"November", L"December",
};

std::wstring_view WeekdayName(Weekday wd, bool shortname) noexcept {
  if (wd < Sunday && wd > Saturday) {
    return L"";
  }
  return shortname ? shortDayNames[static_cast<int>(wd)] : longDayNames[static_cast<int>(wd)];
}

std::wstring_view MonthName(Month mon, bool shortname) noexcept {
  if (mon < January && mon > December) {
    return L"";
  }
  return shortname ? shortMonthNames[static_cast<int>(mon) - 1] : longMonthNames[static_cast<int>(mon) - 1];
}

// Unix epoch
constexpr std::int_least64_t daysSinceEpoch(std::int_least64_t Y) {
  Y--;
  return Y * 365 + Y / 4 - Y / 100 + Y / 400 - unixEpochDays;
}

namespace time_internal {

/* 2000-03-01 (mod 400 year, immediately after feb29 */
constexpr auto LEAPOCH = (946684800LL + 86400 * (31 + 29));

bool MakeDateTime(int64_t second, DateTime &dt) {
  static constexpr const uint8_t days_in_month[] = {31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29};
  auto dsec = second % secondsPerDay;
  dt.hour = static_cast<std::int_least8_t>(dsec / secondsPerHour);                        // 28937
  dt.minute = static_cast<std::int_least8_t>((dsec % secondsPerHour) / secondsPerMinute); // 137
  dt.second = static_cast<std::int_least8_t>(dsec % secondsPerMinute);
  auto days = (second - LEAPOCH) / secondsPerDay;
  auto qccycles = days / daysPer400Years;
  auto remdays = days % daysPer400Years;
  if (remdays < 0) {
    remdays += daysPer400Years;
    qccycles--;
  }
  auto ccycles = remdays / daysPer100Years;
  if (ccycles == 4) {
    ccycles--;
  }
  auto qcycles = remdays / daysPer4Years;
  if (qcycles == 25) {
    qcycles--;
  }
  remdays -= qcycles * daysPer4Years;
  auto remyears = remdays / 365;
  if (remyears == 4) {
    remyears--;
  }
  remdays -= remyears * 365;
  auto leap = remyears == 0 && (qcycles != 0 || ccycles == 0);
  auto yday = remdays + 32 + 28 + leap ? 1 : 0;
  if (yday >= 365 + leap ? 1 : 0) {
    yday -= 365 + leap;
  }
  auto years = remyears + 4 * qcycles + 100 * ccycles + 400LL * qccycles;
  int months = 0;
  for (; days_in_month[months] <= remdays; months++) {
    remdays -= days_in_month[months];
  }

  if (months >= 10) {
    months -= 12;
    years++;
  }

  if (years + 100 > INT_MAX || years + 100 < INT_MIN) {
    return false;
  }
  // https://en.cppreference.com/w/c/chrono/tm
  // Port from musl.
  // years since 1900
  dt.year = years + 100 + 1900;
  // months since January – [0, 11]
  dt.month = static_cast<Month>(months + 2 + 1);
  dt.day = static_cast<std::int_least8_t>(remdays + 1);
  return true;
}
}; // namespace time_internal

// From bela::Time
DateTime::DateTime(bela::Time t, std::int_least32_t tz) noexcept {
  tzoffset = tz;
  auto parts = bela::Split(t);
  nsec = parts.nsec;
  time_internal::MakeDateTime(parts.sec, *this);
}

bela::Time DateTime::Time() const noexcept {
  if (second < 0 || second > 59 || minute > 59 || minute < 0 || hour < 0 || hour > 23 || month < 1 && month > 12 ||
      year < 1970) {
    return bela::UnixEpoch();
  }
  auto leapYear = IsLeapYear(year);
  auto mondays = leapYear ? LeapMonths[month - 1] : Months[month - 1];
  if (day < 1 || day > mondays) {
    return bela::UnixEpoch();
  }
  auto days = daysSinceEpoch(year);
  for (auto i = 1; i < month; i++) {
    days += Months[i - 1];
  }
  if (month > 2 && leapYear) {
    days++;
  }

  days += day - 1;
  auto rep_hi = days * secondsPerDay + hour * 3600 + minute * 60 + second;
  rep_hi += tzoffset;

  const auto d = time_internal::MakeDuration(rep_hi, nsec * time_internal::kTicksPerNanosecond);
  return time_internal::FromUnixDuration(d);
}

} // namespace bela