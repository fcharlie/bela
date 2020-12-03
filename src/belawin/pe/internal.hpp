///
#ifndef BELA_PE_INTERNAL_HPP
#define BELA_PE_INTERNAL_HPP
#include <bela/pe.hpp>

namespace bela::pe {

inline bool PositionAt(HANDLE fd, uint64_t pos, bela::error_code &ec) {
  auto li = *reinterpret_cast<LARGE_INTEGER *>(&pos);
  LARGE_INTEGER oli{0};
  if (SetFilePointerEx(fd, li, &oli, SEEK_SET) != TRUE) {
    ec = bela::make_error_code(L"SetFilePointerEx: ");
    return false;
  }
  return true;
}

inline ssize_t Read(HANDLE fd, void *buffer, size_t len, bela::error_code &ec) {
  DWORD drSize = {0};
  if (::ReadFile(fd, buffer, static_cast<DWORD>(len), &drSize, nullptr) != TRUE) {
    ec = bela::make_system_error_code(L"ReadFile: ");
    return -1;
  }
  return static_cast<ssize_t>(len);
}

inline ssize_t ReadAt(HANDLE fd, void *buffer, size_t len, int64_t pos, bela::error_code &ec) {
  auto li = *reinterpret_cast<LARGE_INTEGER *>(&pos);
  LARGE_INTEGER oli{0};
  if (SetFilePointerEx(fd, li, &oli, SEEK_SET) != TRUE) {
    ec = bela::make_error_code(L"SetFilePointerEx: ");
    return -1;
  }
  return Read(fd, buffer, len, ec);
}

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

struct ImportDirectory {
  uint32_t OriginalFirstThunk;
  uint32_t TimeDateStamp;
  uint32_t ForwarderChain;
  uint32_t Name;
  uint32_t FirstThunk;

  std::string DllName;
};

struct ImportDelayDirectory {
  uint32_t Attributes;
  uint32_t DllNameRVA;
  uint32_t ModuleHandleRVA;
  uint32_t ImportAddressTableRVA;
  uint32_t ImportNameTableRVA;
  uint32_t BoundImportAddressTableRVA;
  uint32_t UnloadInformationTableRVA;
  uint32_t TimeDateStamp;

  std::string DllName;
};

std::string sectionFullName(SectionHeader32 &sh, StringTable &st);
bool readRelocs(Section &sec, HANDLE fd);
bool readSectionData(std::vector<char> &data, const Section &sec, HANDLE fd);
bool readStringTable(FileHeader *fh, HANDLE fd, StringTable &table, bela::error_code &ec);
} // namespace bela::pe

#endif