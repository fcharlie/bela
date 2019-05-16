///
#include <bela/fmt.hpp>
#include <bela/stdwriter.hpp>

int wmain(int argc, wchar_t **argv) {
  auto ux = "\xf0\x9f\x98\x81 UTF-8 text"; // force encode UTF-8
  wchar_t wx[] = L"Engine \xD83D\xDEE0";
  bela::FPrintF(stderr, L"%s %s %s %d\n", argv[0], wx, ux,__cplusplus);
  return 0;
}