///
#include "internal.hpp"

namespace bela::pe {
void StringTable::MoveFrom(StringTable &&other) {
  if (data != nullptr) {
    HeapFree(GetProcessHeap(), 0, data);
  }
  data = other.data;
  length = other.length;
  other.data = nullptr;
  other.length = 0;
}

StringTable::~StringTable() { HeapFree(GetProcessHeap(), 0, data); }

std::optional<std::wstring> StringTable::String(uint32_t start, bela::error_code &ec) {
  if (start < 4) {
    ec = bela::make_error_code(1, L"offset ", start, L" is before the start of string table");
    return std::nullopt;
  }
  start -= 4;
  if (static_cast<size_t>(start) > length) {
    ec = bela::make_error_code(1, L"offset ", start, L" is beyond the end of string table");
    return std::nullopt;
  }
  std::string_view sv;
  auto begin = data + start;
  auto len = length - start;
  auto p = memchr(data + start, 0, length - start);
  if (p == nullptr) {
    sv = {reinterpret_cast<const char *>(begin), len};
  } else {
    sv = {reinterpret_cast<const char *>(begin), len - (reinterpret_cast<const uint8_t *>(p) - begin)};
  }
  return std::make_optional(bela::fromascii(sv));
}

bool readStringTable(FileHeader *fh, FILE *fd, StringTable &table, bela::error_code &ec) {
  if (table.data != nullptr) {
    ec = bela::make_error_code(L"StringTable data not nullptr");
    return false;
  }
  if (fh->PointerToSymbolTable <= 0) {
    // table nullptr
    return true;
  }
  auto offset = fh->PointerToSymbolTable + COFFSymbolSize * fh->NumberOfSymbols;
  if (auto eno = _fseeki64(fd, static_cast<long long>(offset), SEEK_SET); eno != 0) {
    ec = bela::make_stdc_error_code(eno, L"fail to seek to string table: ");
    return false;
  }
  uint32_t l = 0;
  if (auto eno = fread(&l, sizeof(l), 1, fd); eno != 1) {
    ec = bela::make_stdc_error_code(ferror(fd), L"fail to read string table length: ");
    return false;
  }
  l = bela::swaple(l);
  if (l <= 4) {
    return false;
  }
  l -= 4;
  if (table.data = reinterpret_cast<uint8_t *>(HeapAlloc(GetProcessHeap(), 0, l)); table.data == nullptr) {
    ec = bela::make_system_error_code(L"fail to allocate string table memory: ");
    return false;
  }
  if (auto n = fread(table.data, 1, l, fd); n != l) {
    ec = bela::make_stdc_error_code(ferror(fd), L"fail to read string table: ");
    return false;
  }
  table.length = l;
  return true;
}
} // namespace bela::pe