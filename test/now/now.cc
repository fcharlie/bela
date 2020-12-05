//
#include <bela/time.hpp>
#include <bela/terminal.hpp>
#include <bela/base.hpp>

int wmain() {
  auto now = bela::Now();
  FILETIME ft = {0};
  GetSystemTimePreciseAsFileTime(&ft);
  bela::FPrintF(stderr, L"now:  %d\n", bela::ToUnixSeconds(now));
  auto tickNow = bela::FromWindowsPreciseTime(*reinterpret_cast<int64_t *>(&ft));
  bela::FPrintF(stderr, L"tick: %d\n", bela::ToUnixSeconds(tickNow));
  return 0;
}