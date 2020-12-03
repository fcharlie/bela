//
#include "internal.hpp"
#include <charconv>

namespace bela::pe {

std::string sectionFullName(SectionHeader32 &sh, StringTable &st) {
  if (sh.Name[0] != '/') {
    return std::string(cstring_view(sh.Name, sizeof(sh.Name)));
  }
  auto slen = cstring_view(sh.Name + 1, sizeof(sh.Name) - 1);
  uint32_t offset = 0;
  if (auto result = std::from_chars(slen.data(), slen.data() + slen.size(), offset); result.ec != std::errc{}) {
    return "";
  }
  bela::error_code ec;
  return st.String(offset, ec);
}

bool readRelocs(Section &sec, HANDLE fd) {
  if (sec.Header.NumberOfRelocations == 0) {
    return true;
  }
  bela::error_code ec;
  if (!PositionAt(fd, static_cast<int64_t>(sec.Header.PointerToRelocations), ec)) {
    return false;
  }
  sec.Relocs.resize(sec.Header.NumberOfRelocations);
  if (Read(fd, sec.Relocs.data(), sizeof(Reloc) * sec.Header.NumberOfRelocations, ec) !=
      sizeof(Reloc) * sec.Header.NumberOfRelocations) {
    return false;
  }
  if constexpr (bela::IsBigEndian()) {
    for (auto &reloc : sec.Relocs) {
      reloc.VirtualAddress = bela::swaple(reloc.VirtualAddress);
      reloc.SymbolTableIndex = bela::swaple(reloc.SymbolTableIndex);
      reloc.Type = bela::swaple(reloc.Type);
    }
  }
  return true;
}

bool readSectionData(std::vector<char> &data, const Section &sec, HANDLE fd) {
  bela::error_code ec;
  if (!PositionAt(fd, static_cast<int64_t>(sec.Header.Offset), ec)) {
    return false;
  }
  data.resize(sec.Header.Size);
  return Read(fd, data.data(), sec.Header.Size, ec) == static_cast<ssize_t>(sec.Header.Size);
}

} // namespace bela::pe