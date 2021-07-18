///
#include "internal.hpp"

namespace bela::pe {
// manifest

std::wstring_view ResourceTypeName(ULONG_PTR type) {
  (void)type;
  return L"";
}

std::optional<Version> File::FileVersion(bela::error_code &ec) const {
  auto dd = getDataDirectory(IMAGE_DIRECTORY_ENTRY_RESOURCE);
  if (dd == nullptr) {
    return std::nullopt;
  }
  auto sec = getSection(dd);
  if (sec == nullptr) {
    return std::nullopt;
  }
  auto offset = static_cast<int64_t>(sec->Offset);
  IMAGE_RESOURCE_DIRECTORY ird;
  if (!ReadAt(&ird, sizeof(IMAGE_RESOURCE_DIRECTORY), offset, ec)) {
    return std::nullopt;
  }
  return std::nullopt;
}

bool File::LookupResource(bela::error_code &ec) const {
  //
  return true;
}

} // namespace bela::pe