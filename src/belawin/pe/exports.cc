// exports
#include "internal.hpp"
#include <algorithm>

namespace bela::pe {
bool File::LookupExports(std::vector<ExportedSymbol> &exports, bela::error_code &ec) const {
  auto exd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT);
  if (exd == nullptr) {
    return true;
  }
  auto ds = getSection(exd);
  if (ds == nullptr) {
    return true;
  }
  std::vector<char> sdata;
  if (!readSectionData(*ds, sdata)) {
    ec = bela::make_error_code(L"unable read section data");
    return false;
  }
  // seek to the virtual address specified in the export data directory
  auto N = exd->VirtualAddress - ds->VirtualAddress;
  std::string_view sdv{sdata.data() + N, sdata.size() - N};
  if (sdv.size() < sizeof(IMAGE_EXPORT_DIRECTORY)) {
    return true;
  }
  IMAGE_EXPORT_DIRECTORY ied;
  if constexpr (bela::IsLittleEndian()) {
    memcpy(&ied, sdv.data(), sizeof(IMAGE_EXPORT_DIRECTORY));
  } else {
    auto cied = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY *>(sdv.data());
    ied.Characteristics = bela::fromle(cied->Characteristics);
    ied.TimeDateStamp = bela::fromle(cied->TimeDateStamp);
    ied.MajorVersion = bela::fromle(cied->MajorVersion);
    ied.MinorVersion = bela::fromle(cied->MinorVersion);
    ied.Name = bela::fromle(cied->Name);
    ied.Base = bela::fromle(cied->Base);
    ied.NumberOfFunctions = bela::fromle(cied->NumberOfFunctions);
    ied.NumberOfNames = bela::fromle(cied->NumberOfNames);
    ied.AddressOfFunctions = bela::fromle(cied->AddressOfFunctions);       // RVA from base of image
    ied.AddressOfNames = bela::fromle(cied->AddressOfNames);               // RVA from base of image
    ied.AddressOfNameOrdinals = bela::fromle(cied->AddressOfNameOrdinals); // RVA from base of image
  }
  if (ied.NumberOfNames == 0) {
    return true;
  }
  auto ordinalBase = static_cast<uint16_t>(ied.Base);
  exports.resize(ied.NumberOfNames);
  if (ied.AddressOfNameOrdinals > ds->VirtualAddress &&
      ied.AddressOfNameOrdinals < ds->VirtualAddress + ds->VirtualSize) {
    auto L = ied.AddressOfNameOrdinals - ds->VirtualAddress;
    auto sv = std::string_view{sdata.data() + L, sdata.size() - L};
    if (sv.size() > exports.size() * 2) {
      for (size_t i = 0; i < exports.size(); i++) {
        exports[i].Ordinal = bela::cast_fromle<uint16_t>(sv.data() + i * 2) + ordinalBase;
        exports[i].Hint = static_cast<int>(i);
      }
    }
  }
  if (ied.AddressOfNames > ds->VirtualAddress && ied.AddressOfNames < ds->VirtualAddress + ds->VirtualSize) {
    auto N = ied.AddressOfNames - ds->VirtualAddress;
    auto sv = std::string_view{sdata.data() + N, sdata.size() - N};
    if (sv.size() >= exports.size() * 4) {
      for (size_t i = 0; i < exports.size(); i++) {
        auto start = bela::cast_fromle<uint32_t>(sv.data() + i * 4) - ds->VirtualAddress;
        exports[i].Name = getString(sdata, start);
      }
    }
  }
  if (ied.AddressOfFunctions > ds->VirtualAddress && ied.AddressOfFunctions < ds->VirtualAddress + ds->VirtualSize) {
    auto L = ied.AddressOfFunctions - ds->VirtualAddress;
    for (size_t i = 0; i < exports.size(); i++) {
      auto sv = std::string_view{sdata.data() + L, sdata.size() - L};
      if (sv.size() > static_cast<size_t>(exports[i].Ordinal * 4 + 4)) {
        exports[i].Address =
            bela::cast_fromle<uint32_t>(sv.data() + static_cast<int>(exports[i].Ordinal - ordinalBase) * 4);
      }
    }
  }
  std::sort(exports.begin(), exports.end(), [](const ExportedSymbol &a, const ExportedSymbol &b) -> bool {
    //
    return a.Ordinal < b.Ordinal;
  });
  return true;
}

} // namespace bela::pe