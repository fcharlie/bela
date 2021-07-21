///
#ifndef BELA_PE_INTERNAL_HPP
#define BELA_PE_INTERNAL_HPP
#include <bela/pe.hpp>

namespace bela::pe {

inline std::string_view cstring_view(const char *data, size_t len) {
  std::string_view sv{data, len};
  if (auto p = sv.find('\0'); p != std::string_view::npos) {
    return sv.substr(0, p);
  }
  return sv;
}

inline std::string_view cstring_view(const uint8_t *data, size_t len) {
  return cstring_view(reinterpret_cast<const char *>(data), len);
}

// getString extracts a string from symbol string table.
inline std::string getString(std::vector<char> &section, int start) {
  if (start < 0 || static_cast<size_t>(start) >= section.size()) {
    return "";
  }
  for (auto end = static_cast<size_t>(start); end < section.size(); end++) {
    if (section[end] == 0) {
      return std::string(section.data() + start, end - start);
    }
  }
  return "";
}

inline uint16_t getFunctionHit(std::vector<char> &section, int start) {
  if (start < 0 || static_cast<size_t>(start + 2) > section.size()) {
    return 0;
  }
  return bela::cast_fromle<uint16_t>(section.data() + start);
}

struct image_import_descriptor {
  uint32_t OriginalFirstThunk;
  uint32_t TimeDateStamp;
  uint32_t ForwarderChain;
  uint32_t Name;
  uint32_t FirstThunk;
};

struct image_delayload_descriptor {
  uint32_t Attributes;
  uint32_t DllNameRVA;
  uint32_t ModuleHandleRVA;
  uint32_t ImportAddressTableRVA;
  uint32_t ImportNameTableRVA;
  uint32_t BoundImportAddressTableRVA;
  uint32_t UnloadInformationTableRVA;
  uint32_t TimeDateStamp;
};
} // namespace bela::pe

#endif