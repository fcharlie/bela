//
#include <hazel/hazel.hpp>
#include <bela/mapview.hpp>
#include <bela/path.hpp>
#include "ina/hazelinc.hpp"

namespace hazel {

bool File::NewFile(std::wstring_view file, bela::error_code &ec) {
  if (fd != nullfile_t) {
    ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
    return false;
  }
  fd = CreateFileW(file.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL, nullptr);
  if (fd == nullfile_t) {
    ec = bela::make_system_error_code();
    return false;
  }
  if (auto path = bela::RealPathByHandle(fd, ec); path) {
    fullpath = std::move(*path);
  }
  return true;
}

bool File::ReadAt(bela::Buffer &b, size_t len, uint64_t pos, bela::error_code &ec) {
  LARGE_INTEGER li;
  li.QuadPart = pos;
  LARGE_INTEGER oli{0};
  if (SetFilePointerEx(fd, li, &oli, SEEK_SET) != TRUE) {
    ec = bela::make_error_code(L"SetFilePointerEx: ");
    return false;
  }
  if (len > b.capacity()) {
    b.grow(bela::align_length(len));
  }
  DWORD dwSize = {0};
  if (ReadFile(fd, b.data(), static_cast<DWORD>(len), &dwSize, nullptr) != TRUE) {
    ec = bela::make_system_error_code(L"ReadFile: ");
    return false;
  }
  b.size() = len;
  return true;
}
typedef hazel::internal::status_t (*lookup_handle_t)(bela::MemView mv, FileAttributeTable &fat);
bool File::Lookup(FileAttributeTable &fat, bela::error_code &ec) {
  bela::Buffer buffer(4096);
  if (!ReadAt(buffer, 0, ec)) {
    return false;
  }
  bela::MemView mv(buffer.data(), buffer.size());
  using namespace hazel::internal;
  constexpr lookup_handle_t handles[] = {
      LookupExecutableFile, //
      LookupArchives,       // 7z ...
      LookupDocs,
      LookupFonts,     //
      LookupShellLink, // shortcut
      LookupZipFamily, // zip
      LookupMedia,     // media
      LookupImages,    // images
      LookupText,
  };
  for (auto h : handles) {
    if (h(mv, fat) == Found) {
      return true;
    }
  }
  return false;
}

} // namespace hazel