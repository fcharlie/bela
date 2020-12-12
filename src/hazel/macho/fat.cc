///
#include <hazel/macho.hpp>

namespace hazel::macho {

bool FatFile::NewFile(std::wstring_view p, bela::error_code &ec) {
  if (fd != INVALID_HANDLE_VALUE) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = CreateFileW(p.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL, nullptr);
  if (fd == INVALID_HANDLE_VALUE) {
    ec = bela::make_system_error_code();
    return false;
  }
  needClosed = true;
  return ParseFile(ec);
}

bool FatFile::NewFile(HANDLE fd_, int64_t sz, bela::error_code &ec) {
  if (fd != INVALID_HANDLE_VALUE) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = fd_;
  size = sz;
  return ParseFile(ec);
}

//
bool FatFile::ParseFile(bela::error_code &ec) {
  uint8_t ident[4] = {0};
  if (!ReadAt(ident, sizeof(ident), 0, ec)) {
    return false;
  }
  if (bela::readbe<uint32_t>(ident) != MagicFat) {
    // See if this is a Mach-O file via its magic number. The magic
    // must be converted to little endian first though.
    auto lm = bela::readle<uint32_t>(ident);
    if (lm == Magic32 || lm == Magic64) {
      ec = bela::make_error_code(ErrNotFat, L"not a fat Mach-O file");
      return false;
    }
    ec = bela::make_error_code(1, L"macho: bad magic number ['", static_cast<int>(ident[0]), L"', '",
                               static_cast<int>(ident[1]), L"', '", static_cast<int>(ident[2]), L"', '",
                               static_cast<int>(ident[3]), L"']");
    return false;
  }
  auto offset = 4ll;
  uint32_t narch{0};
  if (!ReadFull(&narch, sizeof(narch), ec)) {
    return false;
  }
  narch = bela::swapbe(narch);
  offset += 4;
  if (narch < 1) {
    ec = bela::make_error_code(L"file contains no images");
    return false;
  }
  return false;
}

} // namespace hazel::macho