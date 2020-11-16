//
#include "internal.hpp"

namespace bela::pe {

void File::Free() {
  if (fd != nullptr) {
    fclose(fd);
    fd = nullptr;
  }
}

void File::FileMove(File &&other) {
  Free();
  fd = other.fd;
  other.fd = nullptr;
}

std::optional<File> File::NewFile(std::wstring_view p, bela::error_code &ec) {
  //
  return std::nullopt;
}
} // namespace bela::pe