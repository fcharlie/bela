///
#include "internal.hpp"

namespace bela::pe {
typedef enum ReplacesGeneralNumericDefines {
// Directory entry macro for CLR data.
#ifndef IMAGE_DIRECTORY_ENTRY_COMHEADER
  IMAGE_DIRECTORY_ENTRY_COMHEADER = 14,
#endif // IMAGE_DIRECTORY_ENTRY_COMHEADER
} ReplacesGeneralNumericDefines;
#define STORAGE_MAGIC_SIG 0x424A5342 // BSJB

bool File::LookupClrVersion(std::string &ver, bela::error_code &ec) const {
  auto clrd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_COMHEADER);
  if (clrd == nullptr) {
    return true;
  }
  auto sec = getSection(clrd);
  if (sec == nullptr) {
    return true;
  }
  std::vector<char> sdata;
  if (!readSectionData(*sec, sdata)) {
    ec = bela::make_error_code(L"unable read section data");
    return false;
  }
  auto N = clrd->VirtualAddress - sec->VirtualAddress;
  std::string_view sv{sdata.data() + N, sdata.size() - N};
  if (sv.size() < sizeof(IMAGE_COR20_HEADER)) {
    return false;
  }
  auto cr = reinterpret_cast<const IMAGE_COR20_HEADER *>(sv.data());
  N = cr->MetaData.VirtualAddress - sec->VirtualAddress;
  std::string_view sv2{sdata.data() + N, sdata.size() - N};
  if (sv.size() < sizeof(STORAGESIGNATURE)) {
    return false;
  }
  auto d = reinterpret_cast<const STORAGESIGNATURE *>(sv2.data());
  STORAGESIGNATURE ssi = {0};
  if (bela::fromle(d->lSignature) != STORAGE_MAGIC_SIG) {
    return false;
  }
  ver = getString(sdata, N + sizeof(STORAGESIGNATURE));
  return true;
}

} // namespace bela::pe