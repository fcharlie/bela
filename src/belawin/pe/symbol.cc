//
#include "internal.hpp"

namespace bela::pe {

bool readCOFFSymbols(FileHeader *fh, FILE *fd, std::vector<COFFSymbol> &symbols, bela::error_code &ec) {
  if (fh->PointerToSymbolTable == 0 || fh->NumberOfSymbols <= 0) {
    return true;
  }
  if (auto eno = _fseeki64(fd, int64_t(fh->PointerToSymbolTable), SEEK_SET); eno != 0) {
    ec = bela::make_stdc_error_code(eno, L"fail to seek to symbol table: ");
    return false;
  }
  symbols.resize(fh->NumberOfSymbols);
  if (fread(symbols.data(), 1, sizeof(COFFSymbol) * fh->NumberOfSymbols, fd) !=
      sizeof(COFFSymbol) * fh->NumberOfSymbols) {
    ec = bela::make_stdc_error_code(ferror(fd), L"fail to read symbol table: ");
    return false;
  }
  for (auto &s : symbols) {
    s.SectionNumber = bela::swaple(s.SectionNumber);
    s.Type = bela::swaple(s.Type);
    s.Value = bela::swaple(s.Value);
  }
  return true;
}

} // namespace bela::pe