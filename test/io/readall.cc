#include <bela/io.hpp>
#include <bela/terminal.hpp>

int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s file\n", argv[0]);
    return 1;
  }
  std::wstring buf;
  bela::error_code ec;
  if (!bela::io::ReadFile(argv[1], buf, ec)) {
    bela::FPrintF(stderr, L"open file %s\n", ec.message);
    return 1;
  }
  bela::FPrintF(stderr, L"File read utf16 chars %d\n", buf.size());

  return 0;
}