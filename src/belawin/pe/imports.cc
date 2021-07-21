// imports
#include "internal.hpp"
#include <algorithm>

namespace bela::pe {
bool File::LookupImports(FunctionTable::symbols_map_t &sm, bela::error_code &ec) const {
  auto idd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (idd == nullptr) {
    return true;
  }
  auto ds = getSection(idd);
  if (ds == nullptr) {
    return true;
  }
  auto sdata = readSectionData(*ds, ec);
  if (!sdata) {
    return false;
  }
  auto offset = idd->VirtualAddress - ds->VirtualAddress;
  std::vector<image_import_descriptor> ida;
  for (;;) {
    auto dt = sdata->direct_cast<IMAGE_IMPORT_DESCRIPTOR>(offset);
    if (dt == nullptr) {
      break;
    }
    offset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    image_import_descriptor id;
    if (id.OriginalFirstThunk = bela::fromle(dt->OriginalFirstThunk); id.OriginalFirstThunk == 0) {
      break;
    }
    id.TimeDateStamp = bela::fromle(dt->TimeDateStamp);
    id.ForwarderChain = bela::fromle(dt->ForwarderChain);
    id.Name = bela::fromle(dt->Name);
    id.FirstThunk = bela::fromle(dt->FirstThunk);
    ida.emplace_back(std::move(id));
  }
  for (auto &dt : ida) {
    dt.DllName = sdata->cstring_view(dt.Name - ds->VirtualAddress);
    std::vector<Function> functions;
    size_t funcOffset = dt.OriginalFirstThunk - ds->VirtualAddress;
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
            sdata->cstring_view(static_cast<size_t>(static_cast<uint64_t>(va) - ds->VirtualAddress + 2)),
            sdata->cast_fromle<uint16_t>(static_cast<size_t>(static_cast<uint64_t>(va) - ds->VirtualAddress)));
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
        functions.emplace_back(sdata->cstring_view(static_cast<size_t>(va - ds->VirtualAddress + 2)),
                               sdata->cast_fromle<uint16_t>(static_cast<size_t>(va - ds->VirtualAddress)));
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