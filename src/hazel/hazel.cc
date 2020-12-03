//
#include <type_traits>
#include <hazel/hazel.hpp>
#include <bela/mapview.hpp>
#include <bela/path.hpp>
#include "ina/hazelinc.hpp"

namespace hazel {

typedef hazel::internal::status_t (*lookup_handle_t)(bela::MemView mv, FileAttributeTable &fat);

bool LookupFile(hazel::io::ReaderAt &ra, FileAttributeTable &fat, bela::error_code &ec) {
  uint8_t buffer[4096];
  auto outlen = ra.ReadAt(buffer, sizeof(buffer), 0, ec);
  if (outlen == -1) {
    return false;
  }
  bela::MemView mv(buffer, static_cast<size_t>(outlen));
  using namespace hazel::internal;
  constexpr lookup_handle_t handles[] = {
      LookupExecutableFile, //
      LookupArchives,       // 7z ...
      LookupDocs,
      LookupFonts,     //
      LookupShellLink, // shortcut
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