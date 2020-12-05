///
#include <bela/datetime.hpp>

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
// Offsets to convert between internal and absolute or Unix times.
constexpr int64_t absoluteToInternal = (absoluteZeroYear - internalYear) * 365.2425 * secondsPerDay;
constexpr int64_t internalToAbsolute = -absoluteToInternal;
constexpr int64_t unixToInternal = (1969 * 365 + 1969 / 4 - 1969 / 100 + 1969 / 400) * secondsPerDay;
constexpr int64_t internalToUnix = -unixToInternal;
constexpr int64_t wallToInternal = (1884 * 365 + 1884 / 4 - 1884 / 100 + 1884 / 400) * secondsPerDay;
constexpr int64_t internalToWall = -wallToInternal;
constexpr int32_t daysBefore[] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

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

uint64_t daysSinceEpoch(int year) {
  //
  return 0;
}

// From bela::Time
DateTime::DateTime(bela::Time t) {
  //
}

// ABSL_DLL extern const char RFC3339_full[] = "%Y-%m-%d%ET%H:%M:%E*S%Ez";
// ABSL_DLL extern const char RFC3339_sec[] = "%Y-%m-%d%ET%H:%M:%S%Ez";

// ABSL_DLL extern const char RFC1123_full[] = "%a, %d %b %E4Y %H:%M:%S %z";
// ABSL_DLL extern const char RFC1123_no_wday[] = "%d %b %E4Y %H:%M:%S %z";
std::wstring DateTime::Format(bool full) {
  //
  return L"";
}

} // namespace bela