///
#include <bela/datetime.hpp>

namespace bela {
// SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-gettimezoneinformation
void GetTimeZoneInformationDump() {
  TIME_ZONE_INFORMATION tz;
  GetTimeZoneInformation(&tz);
}

} // namespace bela