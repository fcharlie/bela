///
#include "internal.hpp"

namespace bela::pe {
// manifest

std::wstring_view ResourceTypeName(ULONG_PTR type) {
  (void)type;
  return L"";
}

std::optional<Version> File::FileVersion(bela::error_code &ec) const {
  uint32_t ddlen = 0;
  const DataDirectory *resrd = nullptr;
  if (is64bit) {
    ddlen = oh.NumberOfRvaAndSizes;
    resrd = &(oh.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
  } else {
    auto oh3 = reinterpret_cast<const OptionalHeader32 *>(&oh);
    ddlen = oh3->NumberOfRvaAndSizes;
    resrd = &(oh3->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
  }
  if (ddlen < IMAGE_DIRECTORY_ENTRY_IMPORT + 1 || resrd->VirtualAddress == 0) {
    return std::nullopt;
  }
  const Section *ds = nullptr;
  for (const auto &sec : sections) {
    if (sec.Header.VirtualAddress <= resrd->VirtualAddress &&
        resrd->VirtualAddress < sec.Header.VirtualAddress + sec.Header.VirtualSize) {
      ds = &sec;
    }
  }
  if (ds == nullptr) {
    return std::nullopt;
  }
  auto offset = static_cast<int64_t>(ds->Header.Offset);
  IMAGE_RESOURCE_DIRECTORY ird;
  if (!ReadAt(&ird, sizeof(IMAGE_RESOURCE_DIRECTORY), offset, ec)) {
    return std::nullopt;
  }
  return std::nullopt;
}

bool File::LookupResource(bela::error_code &ec) const {
  //
  return true;
}

} // namespace bela::pe