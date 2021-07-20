// imports
#include "internal.hpp"
#include <algorithm>

namespace bela::pe {
// lookupImports32
bool File::lookupImports32(FunctionTable::symbols_map_t &sm, bela::error_code &ec) const {
  auto idd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (idd == nullptr) {
    return true;
  }
  auto ds = getSection(idd);
  if (ds == nullptr) {
    return true;
  }
  std::vector<char> sdata;
  if (!readSectionData(*ds, sdata)) {
    ec = bela::make_error_code(L"unable read section data");
    return false;
  }
  // seek to the virtual address specified in the import data directory
  auto N = idd->VirtualAddress - ds->VirtualAddress;
  std::string_view sv{sdata.data() + N, sdata.size() - N};
  std::vector<ImportDirectory> ida;
  while (sv.size() > 20) {
    const auto dt = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>(sv.data());
    sv.remove_prefix(20);
    ImportDirectory id;
    id.OriginalFirstThunk = bela::fromle(dt->OriginalFirstThunk);
    id.TimeDateStamp = bela::fromle(dt->TimeDateStamp);
    id.ForwarderChain = bela::fromle(dt->ForwarderChain);
    id.Name = bela::fromle(dt->Name);
    id.FirstThunk = bela::fromle(dt->FirstThunk);
    if (id.OriginalFirstThunk == 0) {
      break;
    }
    ida.emplace_back(std::move(id));
  }
  for (auto &dt : ida) {
    dt.DllName = getString(sdata, int(dt.Name - ds->VirtualAddress));
    auto T = dt.OriginalFirstThunk == 0 ? dt.FirstThunk : dt.OriginalFirstThunk;
    if (T < ds->VirtualAddress) {
      break;
    }
    auto N = T - ds->VirtualAddress;
    std::string_view d{sdata.data() + N, sdata.size() - N};
    std::vector<Function> functions;
    while (d.size() >= sizeof(uint32_t)) {
      auto va = bela::cast_fromle<uint32_t>(d.data());
      d.remove_prefix(4);
      if (va == 0) {
        break;
      }
      // IMAGE_ORDINAL_FLAG32
      if ((va & IMAGE_ORDINAL_FLAG32) > 0) {
        auto ordinal = IMAGE_ORDINAL32(va);
        functions.emplace_back("", 0, static_cast<int>(ordinal));
        // is Ordinal
        // TODO add dynimport ordinal support.
        // ord := va&0x0000FFFF
      } else {
        auto fn = getString(sdata, static_cast<int>(va) - ds->VirtualAddress + 2);
        auto hit = getFunctionHit(sdata, static_cast<int>(static_cast<uint32_t>(va)) - ds->VirtualAddress);
        functions.emplace_back(fn, static_cast<int>(hit));
      }
    }
    std::sort(functions.begin(), functions.end(), [](const bela::pe::Function &a, const bela::pe::Function &b) -> bool {
      return a.GetIndex() < b.GetIndex();
    });
    sm.emplace(std::move(dt.DllName), std::move(functions));
  }
  return true;
}
// lookupImports64
bool File::lookupImports64(FunctionTable::symbols_map_t &sm, bela::error_code &ec) const {
  auto idd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (idd == nullptr) {
    return true;
  }
  auto ds = getSection(idd);
  if (ds == nullptr) {
    return true;
  }
  std::vector<char> sdata;
  if (!readSectionData(*ds, sdata)) {
    ec = bela::make_error_code(L"unable read section data");
    return false;
  }
  // seek to the virtual address specified in the import data directory
  auto N = idd->VirtualAddress - ds->VirtualAddress;
  std::string_view sv{sdata.data() + N, sdata.size() - N};
  std::vector<ImportDirectory> ida;
  while (sv.size() > 20) {
    const auto dt = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>(sv.data());
    sv.remove_prefix(20);
    ImportDirectory id;
    id.OriginalFirstThunk = bela::fromle(dt->OriginalFirstThunk);
    id.TimeDateStamp = bela::fromle(dt->TimeDateStamp);
    id.ForwarderChain = bela::fromle(dt->ForwarderChain);
    id.Name = bela::fromle(dt->Name);
    id.FirstThunk = bela::fromle(dt->FirstThunk);
    if (id.OriginalFirstThunk == 0) {
      break;
    }
    ida.emplace_back(std::move(id));
  }
  for (auto &dt : ida) {
    dt.DllName = getString(sdata, int(dt.Name - ds->VirtualAddress));
    auto T = dt.OriginalFirstThunk == 0 ? dt.FirstThunk : dt.OriginalFirstThunk;
    if (T < ds->VirtualAddress) {
      break;
    }
    auto N = T - ds->VirtualAddress;
    std::string_view d{sdata.data() + N, sdata.size() - N};
    std::vector<Function> functions;
    while (d.size() >= sizeof(uint64_t)) {
      auto va = bela::cast_fromle<uint64_t>(d.data());
      d.remove_prefix(8);
      if (va == 0) {
        break;
      }
      // IMAGE_ORDINAL_FLAG64
      if ((va & IMAGE_ORDINAL_FLAG64) > 0) {
        auto ordinal = IMAGE_ORDINAL64(va);
        functions.emplace_back("", 0, static_cast<int>(ordinal));
        // TODO add dynimport ordinal support.
      } else {
        auto fn = getString(sdata, static_cast<int>(static_cast<uint64_t>(va)) - ds->VirtualAddress + 2);
        auto hit = getFunctionHit(sdata, static_cast<int>(static_cast<uint64_t>(va)) - ds->VirtualAddress);
        functions.emplace_back(fn, static_cast<int>(hit));
      }
    }
    std::sort(functions.begin(), functions.end(), [](const bela::pe::Function &a, const bela::pe::Function &b) -> bool {
      return a.GetIndex() < b.GetIndex();
    });
    sm.emplace(std::move(dt.DllName), std::move(functions));
  }
  return true;
}

bool File::LookupImports(FunctionTable::symbols_map_t &sm, bela::error_code &ec) const {
  if (is64bit) {
    return lookupImports64(sm, ec);
  }
  return lookupImports32(sm, ec);
}

} // namespace bela::pe