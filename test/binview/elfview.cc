///
#include <hazel/elf.hpp>
#include <bela/terminal.hpp>

int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s elf-file\n", argv[0]);
    return 1;
  }
  hazel::elf::File file;
  bela::error_code ec;
  if (!file.NewFile(argv[1], ec)) {
    bela::FPrintF(stderr, L"parse elf file %s error: %s\n", argv[1], ec.message);
    return 1;
  }
  for (const auto &s : file.Sections()) {
    bela::FPrintF(stderr, L"%s: %08x (%d) %d %d\n", s.Name, s.Addr, s.Size, s.Entsize, s.Flags);
  }

  return 0;
}