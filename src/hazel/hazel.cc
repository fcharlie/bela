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

typedef hazel::internal::status_t (*lookup_handle_t)(bela::MemView mv, FileAttributeTable &fat);
bool File::Lookup(FileAttributeTable &fat, bela::error_code &ec) {
  uint8_t buffer[4096];
  size_t outlen = 0;
  if (!ReadAt(buffer, sizeof(buffer), 0, outlen, ec)) {
    return false;
  }
  bela::MemView mv(buffer, outlen);
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