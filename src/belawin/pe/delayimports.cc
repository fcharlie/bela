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
  auto sdata = readSectionData(*sec, ec);
  if (!sdata) {
    return false;
  }
  // seek to the virtual address specified in the delay import data directory
  constexpr size_t dslen = sizeof(IMAGE_DELAYLOAD_DESCRIPTOR);
  size_t offset = delay->VirtualAddress - sec->VirtualAddress;
  std::vector<image_delayload_descriptor> ida;
  for (;;) {
    auto dt = sdata->direct_cast<IMAGE_DELAYLOAD_DESCRIPTOR>(offset);
    if (dt == nullptr) {
      break;
    }
    offset += sizeof(IMAGE_DELAYLOAD_DESCRIPTOR);
    image_delayload_descriptor id;
    if (id.ModuleHandleRVA = bela::fromle(dt->ModuleHandleRVA); id.ModuleHandleRVA == 0) {
      break;
    }
    id.Attributes = bela::fromle(dt->Attributes.AllAttributes);
    id.DllNameRVA = bela::fromle(dt->DllNameRVA);
    id.ImportAddressTableRVA = bela::fromle(dt->ImportAddressTableRVA);
    id.ImportNameTableRVA = bela::fromle(dt->ImportNameTableRVA);
    id.BoundImportAddressTableRVA = bela::fromle(dt->BoundImportAddressTableRVA);
    id.UnloadInformationTableRVA = bela::fromle(dt->UnloadInformationTableRVA);
    id.TimeDateStamp = bela::fromle(dt->TimeDateStamp);
    ida.emplace_back(std::move(id));
  }
  for (auto &dt : ida) {
    dt.DllName = sdata->cstring_view(dt.DllNameRVA - sec->VirtualAddress);
    if (dt.ImportNameTableRVA < sec->VirtualAddress || dt.ImportNameTableRVA > sec->VirtualAddress + sec->VirtualSize) {
      break;
    }
    size_t funcOffset = dt.ImportNameTableRVA - sec->VirtualAddress;
    std::vector<Function> functions;
    if (oh.Is64Bit) {
      for (;;) {
        auto va = sdata->cast_fromle<uint64_t>(funcOffset);
        funcOffset += sizeof(uint64_t);
        if (va == 0) {
          break;
        }
        if ((va & IMAGE_ORDINAL_FLAG64) != 0) {
          functions.emplace_back("", 0, static_cast<int>(IMAGE_ORDINAL64(va)));
          continue;
        }
        functions.emplace_back(
            sdata->cstring_view(static_cast<size_t>(static_cast<uint64_t>(va) - sec->VirtualAddress + 2)),
            sdata->cast_fromle<uint16_t>(static_cast<size_t>(static_cast<uint64_t>(va) - sec->VirtualAddress)));
      }
    } else {
      for (;;) {
        auto va = sdata->cast_fromle<uint32_t>(funcOffset);
        funcOffset += sizeof(uint32_t);
        if (va == 0) {
          break;
        }
        if ((va & IMAGE_ORDINAL_FLAG32) != 0) {
          functions.emplace_back("", 0, static_cast<int>(IMAGE_ORDINAL32(va)));
          continue;
        }
        functions.emplace_back(sdata->cstring_view(static_cast<size_t>(va - sec->VirtualAddress + 2)),
                               sdata->cast_fromle<uint16_t>(static_cast<size_t>(va - sec->VirtualAddress)));
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