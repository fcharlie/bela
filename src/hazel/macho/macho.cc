//
#include <hazel/macho.hpp>

namespace hazel::macho {
//

bool File::NewFile(std::wstring_view p, bela::error_code &ec) {
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
bool File::NewFile(HANDLE fd_, int64_t sz, bela::error_code &ec) {
  if (fd != INVALID_HANDLE_VALUE) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = fd_;
  size = sz;
  return ParseFile(ec);
}

bool File::ParseFile(bela::error_code &ec) {
  if (size == bela::SizeUnInitialized) {
    LARGE_INTEGER li;
    if (!GetFileSizeEx(fd, &li) == TRUE) {
      ec = bela::make_system_error_code(L"GetFileSizeEx: ");
      return false;
    }
    size = li.QuadPart;
  }
  uint8_t ident[4] = {0};
  if (!ReadAt(ident, sizeof(ident), 0, ec)) {
    return false;
  }
  auto resolveMagic = [&]() -> bool {
    auto le = bela::readle<uint32_t>(ident);
    if (le == MH_MAGIC) {
      en = bela::endian::Endian::little;
      return true;
    }
    if (le == MH_MAGIC_64) {
      en = bela::endian::Endian::little;
      is64bit = true;
      return true;
    }
    auto be = bela::readle<uint32_t>(ident);
    if (be == MH_MAGIC) {
      en = bela::endian::Endian::big;
      return true;
    }
    if (le == MH_MAGIC_64) {
      en = bela::endian::Endian::big;
      is64bit = true;
      return true;
    }
    return false;
  };
  if (!resolveMagic()) {
    ec = bela::make_error_code(1, L"macho: bad magic number ['", static_cast<int>(ident[0]), L"', '",
                               static_cast<int>(ident[1]), L"', '", static_cast<int>(ident[2]), L"', '",
                               static_cast<int>(ident[3]), L"']");
    return false;
  }

  return false;
}
} // namespace hazel::macho