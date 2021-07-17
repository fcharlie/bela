//
#ifndef BELA_OS_HPP
#define BELA_OS_HPP
#include <cstdint>
#include <type_traits>
#include "base.hpp"

namespace bela::os {
enum FileMode : uint32_t {
  ModeDir = 2147483648,      // d: is a directory
  ModeAppend = 1073741824,   // a: append-only
  ModeExclusive = 536870912, // l: exclusive use
  ModeTemporary = 268435456, // T: temporary file; Plan 9 only
  ModeSymlink = 134217728,   // L: symbolic link
  ModeDevice = 67108864,     // D: device file
  ModeNamedPipe = 33554432,  // p: named pipe (FIFO)
  ModeSocket = 16777216,     // S: Unix domain socket
  ModeSetuid = 8388608,      // u: setuid
  ModeSetgid = 4194304,      // g: setgid
  ModeCharDevice = 4194304,  // c: Unix character device, when ModeDevice is set
  ModeSticky = 1048576,      // t: sticky
  ModeIrregular = 524288,    // ?: non-regular file; nothing else is known about this file

  // Mask for the type bits. For regular files, none will be set.
  ModeType = ModeDir | ModeSymlink | ModeNamedPipe | ModeSocket | ModeDevice | ModeCharDevice | ModeIrregular,

  ModePerm = 0777, // Unix permission bits
};
[[nodiscard]] constexpr FileMode operator&(FileMode L, FileMode R) noexcept {
  using I = std::underlying_type_t<FileMode>;
  return static_cast<FileMode>(static_cast<I>(L) & static_cast<I>(R));
}
[[nodiscard]] constexpr FileMode operator|(FileMode L, FileMode R) noexcept {
  using I = std::underlying_type_t<FileMode>;
  return static_cast<FileMode>(static_cast<I>(L) | static_cast<I>(R));
}

constexpr inline bool IsDir(FileMode m) { return (m & ModeDir) != 0; }
constexpr inline bool IsRegular(FileMode m) { return (m & ModeType) == 0; }
constexpr inline FileMode Perm(FileMode m) { return m & ModePerm; }
constexpr inline FileMode Type(FileMode m) { return m & ModeType; }

// os function

namespace file {
inline int64_t Size(HANDLE fd, bela::error_code &ec) {
  FILE_STANDARD_INFO si;
  if (GetFileInformationByHandleEx(fd, FileStandardInfo, &si, sizeof(si)) != TRUE) {
    ec = bela::make_system_error_code(L"GetFileInformationByHandleEx(): ");
    return bela::SizeUnInitialized;
  }
  return si.EndOfFile.QuadPart;
}

inline int64_t Size(std::wstring_view filename, bela::error_code &ec) {
  WIN32_FILE_ATTRIBUTE_DATA wdata;
  if (GetFileAttributesExW(filename.data(), GetFileExInfoStandard, &wdata) != TRUE) {
    ec = bela::make_system_error_code(L"GetFileAttributesExW(): ");
    return bela::SizeUnInitialized;
  }
  return static_cast<unsigned long long>(wdata.nFileSizeHigh) << 32 | wdata.nFileSizeLow;
}

enum Whence : DWORD {
  SeekStart = FILE_BEGIN,
  SeekCurrent = FILE_CURRENT,
  SeekEnd = FILE_END,
};

inline bool Seek(HANDLE fd, int64_t pos, bela::error_code &ec, Whence whence = SeekStart) {
  LARGE_INTEGER new_pos{.QuadPart = 0};
  if (SetFilePointerEx(fd, *reinterpret_cast<LARGE_INTEGER const *>(&pos), &new_pos, whence) != TRUE) {
    ec = bela::make_system_error_code(L"SetFilePointerEx(): ");
    return false;
  }
  return true;
}

} // namespace file

} // namespace bela::os

#endif