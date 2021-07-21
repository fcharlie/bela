// read pe overlay data
#include "internal.hpp"

namespace bela::pe {
int64_t File::ReadOverlay(std::span<char> overlayData, bela::error_code &ec) const {
  if (size <= overlayOffset) {
    ec = bela::make_error_code(ErrNoOverlay, L"no overlay data");
    return -1;
  }
  if (!bela::os::file::Seek(fd, overlayOffset, ec)) {
    return -1;
  }
  auto minSize = (std::min)(static_cast<int64_t>(overlayData.size()), size - overlayOffset);
  if (!ReadFull(overlayData.data(), minSize, ec)) {
    return -1;
  }
  return minSize;
}

} // namespace bela::pe