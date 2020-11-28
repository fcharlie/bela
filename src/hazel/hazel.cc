//
#include <hazel/hazel.hpp>
#include <bela/mapview.hpp>

namespace hazel {

bool File::NewFile(std::wstring_view file, bela::error_code &ec) {
  if (fd != nullfile_t) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  if (fd = CreateFileW(file.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, nullptr);
      fd == nullfile_t) {
    ec = bela::make_system_error_code();
    return false;
  }
  return true;
}

bool File::Lookup(FileAttributeTable &fat, bela::error_code &ec) {
  if (!SeekStart(0)) {
    ec = bela::make_system_error_code();
    return false;
  }
  uint8_t mbuf[1024];
  DWORD dwSize = 0;
  if (ReadFile(fd, mbuf, 1024, &dwSize, nullptr) != TRUE) {
    ec = bela::make_system_error_code();
    return false;
  }
  bela::MemView mv(mbuf, dwSize);
  return true;
}

} // namespace hazel