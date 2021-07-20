#include "internal.hpp"
#include <algorithm>

namespace bela::pe {
// Delay imports
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#delay-load-import-tables-image-only
bool File::LookupDelayImports(FunctionTable::symbols_map_t &sm, bela::error_code &ec) const {
  auto delay = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
  if (delay == nullptr) {
    return true;
  }
  auto sec = getSection(delay);
  if (sec == nullptr) {
    return true;
  }
  std::vector<char> sdata;
  if (!readSectionData(*sec, sdata)) {
    ec = bela::make_error_code(L"unable read section data");
    return false;
  }
  // seek to the virtual address specified in the delay import data directory
  auto N = delay->VirtualAddress - sec->VirtualAddress;
  std::string_view sdv{sdata.data() + N, sdata.size() - N};

  constexpr size_t dslen = sizeof(IMAGE_DELAYLOAD_DESCRIPTOR);
  std::vector<ImportDelayDirectory> ida;
  while (sdv.size() > dslen) {
    const auto dt = reinterpret_cast<const IMAGE_DELAYLOAD_DESCRIPTOR *>(sdv.data());
    sdv.remove_prefix(dslen);
    ImportDelayDirectory id;
    id.Attributes = bela::fromle(dt->Attributes.AllAttributes);
    id.DllNameRVA = bela::fromle(dt->DllNameRVA);
    id.ModuleHandleRVA = bela::fromle(dt->ModuleHandleRVA);
    id.ImportAddressTableRVA = bela::fromle(dt->ImportAddressTableRVA);
    id.ImportNameTableRVA = bela::fromle(dt->ImportNameTableRVA);
    id.BoundImportAddressTableRVA = bela::fromle(dt->BoundImportAddressTableRVA);
    id.UnloadInformationTableRVA = bela::fromle(dt->UnloadInformationTableRVA);
    id.TimeDateStamp = bela::fromle(dt->TimeDateStamp);
    if (id.ModuleHandleRVA == 0) {
      break;
    }
    ida.emplace_back(std::move(id));
  }
  auto ptrsize = is64bit ? sizeof(uint64_t) : sizeof(uint32_t);
  for (auto &dt : ida) {
    dt.DllName = getString(sdata, int(dt.DllNameRVA - sec->VirtualAddress));
    if (dt.ImportNameTableRVA < sec->VirtualAddress || dt.ImportNameTableRVA > sec->VirtualAddress + sec->VirtualSize) {
      break;
    }
    uint32_t L = dt.ImportNameTableRVA - sec->VirtualAddress;

    std::string_view d{sdata.data() + L, sdata.size() - L};
    std::vector<Function> functions;
    while (d.size() >= ptrsize) {
      if (is64bit) {
        auto va = bela::cast_fromle<uint64_t>(d.data());
        d.remove_prefix(8);
        if (va == 0) {
          break;
        }
        // IMAGE_ORDINAL_FLAG64
        if ((va & 0x8000000000000000) > 0) {
          auto ordinal = IMAGE_ORDINAL64(va);
          functions.emplace_back("", 0, static_cast<int>(ordinal));
          // TODO add dynimport ordinal support.
        } else {
          auto fn = getString(sdata, static_cast<int>(static_cast<uint64_t>(va)) - sec->VirtualAddress + 2);
          auto hit = getFunctionHit(sdata, static_cast<int>(static_cast<uint64_t>(va)) - sec->VirtualAddress);
          functions.emplace_back(fn, static_cast<int>(hit));
        }
      } else {
        auto va = bela::cast_fromle<uint32_t>(d.data());
        d.remove_prefix(4);
        if (va == 0) {
          break;
        }
        // IMAGE_ORDINAL_FLAG32
        if ((va & 0x80000000) > 0) {
          auto ordinal = IMAGE_ORDINAL32(va);
          functions.emplace_back("", 0, static_cast<int>(ordinal));
          // is Ordinal
          // TODO add dynimport ordinal support.
          // ord := va&0x0000FFFF
        } else {
          auto fn = getString(sdata, static_cast<int>(va) - sec->VirtualAddress + 2);
          auto hit = getFunctionHit(sdata, static_cast<int>(static_cast<uint32_t>(va)) - sec->VirtualAddress);
          functions.emplace_back(fn, static_cast<int>(hit));
        }
      }
    }
    std::sort(functions.begin(), functions.end(), [](const bela::pe::Function &a, const bela::pe::Function &b) -> bool {
      return a.GetIndex() < b.GetIndex();
    });
    sm.emplace(std::move(dt.DllName), std::move(functions));
  }
  return true;
}

} // namespace bela::pe