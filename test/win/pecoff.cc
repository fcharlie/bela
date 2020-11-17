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
  bela::FPrintF(stderr,
                L"Is64Bit: %b\nMachine: %d\nCharacteristics: %d\nPointerToSymbolTable: %d\nNumberOfSymbols %d\n",
                file->Is64Bit(), file->Fh().Machine, file->Fh().Characteristics, file->Fh().PointerToSymbolTable,
                file->Fh().NumberOfSymbols);
  std::vector<std::string_view> gstable;
  file->GoStringTable(gstable);
  for (const auto s : gstable) {
    bela::FPrintF(stderr, L"%s\n", s);
  }
  if (file->Is64Bit()) {
    bela::FPrintF(stderr, L"Subsystem %d\n", file->Oh64()->Subsystem);
  }
  return 0;
}