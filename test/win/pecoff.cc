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
  for (const auto &sec : file->Sections()) {
    bela::FPrintF(stderr, L"Section: %s VirtualAddress: %d\n", sec.Header.Name, sec.Header.VirtualAddress);
  }
  bela::pe::symbols_map_t sm;
  file->LookupImports(sm, ec);

  for (const auto &d : sm) {
    bela::FPrintF(stderr, L"\x1b[33mDllName: %s\x1b[0m\n", d.first);
    for (const auto &n : d.second) {
      bela::FPrintF(stderr, L"%s %d\n", n.Name, n.Index);
    }
  }
  return 0;
}