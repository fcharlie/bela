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

std::string StringTable::String(uint32_t start, bela::error_code &ec) const {
  if (start < 4) {
    ec = bela::make_error_code(1, L"offset ", start, L" is before the start of string table");
    return "";
  }
  start -= 4;
  if (static_cast<size_t>(start) > length) {
    ec = bela::make_error_code(1, L"offset ", start, L" is beyond the end of string table");
    return "";
  }
  return std::string(cstring_view(data + start, length - start));
}

bool readStringTable(FileHeader *fh, HANDLE fd, StringTable &table, bela::error_code &ec) {
  if (table.data != nullptr) {
    ec = bela::make_error_code(L"StringTable data not nullptr");
    return false;
  }
  if (fh->PointerToSymbolTable <= 0) {
    // table nullptr
    return true;
  }
  auto offset = fh->PointerToSymbolTable + COFFSymbolSize * fh->NumberOfSymbols;
  if (!PositionAt(fd, static_cast<int64_t>(offset), ec)) {
    return false;
  }
  uint32_t l = 0;
  if (Read(fd, &l, sizeof(l), ec) != 4) {
    ec = bela::make_error_code(1, L"fail to read string table length: ", ec.message);
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
  if (Read(fd, table.data, l, ec) != static_cast<ssize_t>(l)) {
    ec = bela::make_error_code(1, L"fail to read string table: ", ec.message);
    return false;
  }
  table.length = l;
  return true;
}
} // namespace bela::pe