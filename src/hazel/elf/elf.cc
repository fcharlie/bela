///
#include <hazel/hazel.hpp>
#include <hazel/elf.hpp>
#include "elf.h"

namespace hazel::elf {
//
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
  //
  return true;
}

} // namespace hazel::elf