///
#include "internal.hpp"

namespace hazel::elf {

bool File::getSymbols64(uint32_t st, std::vector<Symbol> &syms, bela::Buffer &buffer, bela::error_code &ec) {
  auto symSec = SectionByType(st);
  if (symSec == nullptr) {
    ec = bela::make_error_code(L"no symbol section");
    return false;
  }
  buffer.grow(symSec->Size);
  if (!ReadAt(buffer, size, symSec->Offset, ec)) {
    return false;
  }
  if (buffer.size() != symSec->Size) {
    ec = bela::make_error_code(L"cannot load symbol section");
    return false;
  }
  // if(buffer.size()%)
  return true;
}
bool File::getSymbols32(uint32_t st, std::vector<Symbol> &syms, bela::Buffer &buffer, bela::error_code &ec) {
  //
  return true;
}

} // namespace hazel::elf