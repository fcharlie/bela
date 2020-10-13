//
#include <bela/fs.hpp>

namespace bela::fs {
constexpr auto nohideflags = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY;
bool Remove(std::wstring_view path, bela::error_code &ec) {
  constexpr auto flags = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT;
  constexpr auto shm = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  // 0x00010000
  auto FileHandle = CreateFileW(path.data(), DELETE, shm, nullptr, OPEN_EXISTING, flags, nullptr);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    ec = bela::make_system_error_code(L"CreateFileW ");
    if (ec.code == ERROR_FILE_NOT_FOUND) {
      return true;
    }
    return false;
  }
  auto closer = bela::finally([&] { CloseHandle(FileHandle); });
  struct _File_disposition_info_ex {
    DWORD _Flags;
  };

  _File_disposition_info_ex _Info_ex{0x3};
  constexpr auto _FileDispositionInfoExClass = static_cast<FILE_INFO_BY_HANDLE_CLASS>(21);
  if (SetFileInformationByHandle(FileHandle, _FileDispositionInfoExClass, &_Info_ex, sizeof(_Info_ex)) == TRUE) {
    return true;
  }
  auto e = GetLastError();
  if (e == ERROR_ACCESS_DENIED) {
    SetFileAttributesW(path.data(), GetFileAttributesW(path.data()) & ~nohideflags);
    if (SetFileInformationByHandle(FileHandle, _FileDispositionInfoExClass, &_Info_ex, sizeof(_Info_ex)) == TRUE) {
      return true;
    }
    e = GetLastError();
  }
  switch (e) {
  case ERROR_INVALID_PARAMETER:
    [[fallthrough]];
  case ERROR_INVALID_FUNCTION:
    [[fallthrough]];
  case ERROR_NOT_SUPPORTED:
    break;
  default:
    ec.code = e;
    ec.message = bela::resolve_system_error_message(e);
    return false;
  }
  FILE_DISPOSITION_INFO _Info{/* .Delete= */ TRUE};
  if (SetFileInformationByHandle(FileHandle, FileDispositionInfo, &_Info, sizeof(_Info)) == TRUE) {
    return true;
  }
  e = GetLastError();
  if (ec.code == ERROR_ACCESS_DENIED) {
    SetFileAttributesW(path.data(), GetFileAttributesW(path.data()) & ~nohideflags);
    if (SetFileInformationByHandle(FileHandle, FileDispositionInfo, &_Info, sizeof(_Info)) == TRUE) {
      return true;
    }
    e = GetLastError();
  }
  ec.code = e;
  ec.message = bela::resolve_system_error_message(e);
  return false;
}
} // namespace bela::fs
