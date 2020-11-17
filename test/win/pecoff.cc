///
#include <bela/pe.hpp>
#include <bela/terminal.hpp>

int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s pefile\n", argv[0]);
    return 1;
  }
  bela::error_code ec;
  auto file = bela::pe::NewFile(argv[1], ec);
  if (!file) {
    bela::FPrintF(stderr, L"unable parse pecoff: %s\n", ec.message);
    return 1;
  }
  bela::FPrintF(stderr, L"Is64Bit: %b\n%s\n", file->Is64Bit(), file->Table());
  return 0;
}