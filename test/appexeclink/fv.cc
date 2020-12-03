//
#include <bela/terminal.hpp>
#include <hazel/hazel.hpp>

int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s path\n", argv[0]);
    return 1;
  }
  hazel::io::File file;
  bela::error_code ec;
  if (!file.Open(argv[1], ec)) {
    bela::FPrintF(stderr, L"unable open file %s\n", ec.message);
    return 1;
  }
  hazel::FileAttributeTable fat;
  if (!hazel::LookupFile(file, fat, ec)) {
    bela::FPrintF(stderr, L"unable detect file type %s\n", ec.message);
    return 1;
  }
  bela::FPrintF(stderr, L"file %v %v\n", file.FullPath(), fat.type);
  for (const auto &[k, v] : fat.attributes) {
    bela::FPrintF(stderr, L"%v: %v\n", k, v);
  }
  return 0;
}