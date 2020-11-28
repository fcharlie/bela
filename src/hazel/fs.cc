//
#include <hazel/fs.hpp>
#include <bela/repasepoint.hpp>

namespace hazel::fs {
bool LookupReparsePoint(std::wstring_view file, FileReparsePoint &frp, bela::error_code &ec) {
  bela::Buffer b(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!bela::LookupReparsePoint(file, b, ec)) {
    return false;
  }
  if (b.size() == 0) {
    ec.code = ERROR_NOT_A_REPARSE_POINT;
    ec.message = bela::resolve_system_error_message(ERROR_NOT_A_REPARSE_POINT);
    return false;
  }
  auto p = b.cast<REPARSE_DATA_BUFFER>();
  frp.type = static_cast<reparse_point_t>(p->ReparseTag);
  frp.attributes.emplace(L"TAG", bela::StringCat(L"0x", bela::Hex(frp.type, bela::kZeroPad8)));
  switch (frp.type) {
  case SYMLINK:
    break;
  case MOUNT_POINT:
    break;
  case APPEXECLINK:
    break;
  case LX_SYMLINK:
    break;
  default:
    break;
  }

  return true;
}
} // namespace hazel::fs