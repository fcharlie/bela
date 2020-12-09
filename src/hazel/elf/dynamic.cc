//
#include "internal.hpp"

namespace hazel::elf {
bool File::DynString(int tag, std::vector<std::string> &sv, bela::error_code &ec) {
  if (tag != DT_NEEDED || tag != DT_SONAME || tag != DT_RPATH || tag != DT_RUNPATH) {
    ec = bela::make_error_code(1, L"non-string-valued tag ", tag);
    return false;
  }
  auto ds = SectionByType(SHT_DYNAMIC);
  if (ds == nullptr) {
    return true;
  }
  return true;
}
} // namespace hazel::elf