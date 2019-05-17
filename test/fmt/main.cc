///
#include <bela/stdwriter.hpp>

int wmain(int argc, wchar_t **argv) {
  auto ux = "\xf0\x9f\x98\x81 UTF-8 text \xE3\x8D\xA4"; // force encode UTF-8
  wchar_t wx[] = L"Engine \xD83D\xDEE0 中国";
  bela::FPrintF(stderr, L"Argc: %d Arg0: %s W: %s UTF-8: %s __cplusplus: %d\n",
                argc, argv[0], wx, ux, __cplusplus);
  return 0;
}
