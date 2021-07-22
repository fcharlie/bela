// Bela IO utils
#ifndef BELA_IO_HPP
#define BELA_IO_HPP
#include "base.hpp"
#include "types.hpp"
#include "buffer.hpp"
#include "os.hpp"
#include "path.hpp"

namespace bela::io {
// Size get file size
inline int64_t Size(HANDLE fd, bela::error_code &ec) {
  FILE_STANDARD_INFO si;
  if (GetFileInformationByHandleEx(fd, FileStandardInfo, &si, sizeof(si)) != TRUE) {
    ec = bela::make_system_error_code(L"GetFileInformationByHandleEx(): ");
    return bela::SizeUnInitialized;
  }
  return si.EndOfFile.QuadPart;
}
// Size get file size
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
// Seek pos
inline bool Seek(HANDLE fd, int64_t pos, bela::error_code &ec, Whence whence = SeekStart) {
  LARGE_INTEGER new_pos{.QuadPart = 0};
  if (SetFilePointerEx(fd, *reinterpret_cast<LARGE_INTEGER const *>(&pos), &new_pos, whence) != TRUE) {
    ec = bela::make_system_error_code(L"SetFilePointerEx(): ");
    return false;
  }
  return true;
}

class FD {
private:
  void Free();
  void MoveFrom(FD &&o);

public:
  FD() = default;
  FD(HANDLE fd_, bool needClosed_ = true) : fd(fd_), needClosed(needClosed_) {}
  FD(const FD &) = delete;
  FD &operator=(const FD &) = delete;
  FD(FD &&o) { MoveFrom(std::move(o)); }
  FD &operator=(FD &&o) {
    MoveFrom(std::move(o));
    return *this;
  }
  ~FD() { Free(); }
  FD &Assgin(FD &&o) {
    MoveFrom(std::move(o));
    return *this;
  }
  FD &Assgin(HANDLE fd_, bool needClosed_ = true) {
    Free();
    fd = fd_;
    needClosed = needClosed_;
  }
  explicit operator bool() const { return fd != INVALID_HANDLE_VALUE; }
  const auto address() const { return fd; }
  int64_t Size(bela::error_code &ec) const { return bela::io::Size(fd, ec); }
  // ReadAt reads buffer.size() bytes from the File starting at byte offset pos.
  bool ReadFull(std::span<uint8_t> buffer, bela::error_code &ec) const;
  // ReadFull reads exactly buffer.size() bytes from FD into buffer.
  bool ReadAt(std::span<uint8_t> buffer, int64_t pos, bela::error_code &ec) const;

  template <typename T>
  requires std::is_standard_layout_v<T>
  bool ReadFull(T &t, bela::error_code &ec) const { return ReadFull({reinterpret_cast<uint8_t *>(&t), sizeof(T)}, ec); }
  template <typename T>
  requires std::is_standard_layout_v<T>
  bool ReadVectorFull(std::vector<T> &tv, bela::error_code &ec) const {
    return ReadFull({reinterpret_cast<uint8_t *>(tv.data()), sizeof(T) * tv.size()}, ec);
  }

  template <typename T>
  requires std::is_standard_layout_v<T>
  bool ReadAt(T &t, int64_t pos, bela::error_code &ec) const {
    return ReadAt({reinterpret_cast<uint8_t *>(&t), sizeof(T)}, pos, ec);
  }

  template <typename T>
  requires std::is_standard_layout_v<T>
  bool ReadVectorAt(std::vector<T> &tv, int64_t pos, bela::error_code &ec) const {
    return ReadAt({reinterpret_cast<uint8_t *>(tv.data()), sizeof(T) * tv.size()}, pos, ec);
  }

private:
  HANDLE fd{INVALID_HANDLE_VALUE};
  bool needClosed{true};
};

std::optional<FD> NewFile(std::wstring_view file, bela::error_code &ec);
std::optional<FD> NewFile(std::wstring_view file, DWORD dwDesiredAccess, DWORD dwShareMode,
                          LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                          DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, bela::error_code &ec);

inline bool ReadAt(HANDLE fd, void *buffer, size_t len, int64_t pos, size_t &outlen, bela::error_code &ec) {
  if (!bela::io::Seek(fd, pos, ec)) {
    return false;
  }
  DWORD dwSize = {0};
  if (::ReadFile(fd, buffer, static_cast<DWORD>(len), &dwSize, nullptr) != TRUE) {
    ec = bela::make_system_error_code(L"ReadFile: ");
    return false;
  }
  outlen = static_cast<size_t>(len);
  return true;
}

[[maybe_unused]] constexpr auto MaximumRead = 1024ull * 1024 * 8; // 8MB
[[maybe_unused]] constexpr auto MaximumLineLength = 1024ull * 64; // 64KB
bool ReadFile(std::wstring_view file, std::wstring &out, bela::error_code &ec, uint64_t maxsize = MaximumRead);
bool ReadLine(std::wstring_view file, std::wstring &out, bela::error_code &ec, uint64_t maxline = MaximumLineLength);
inline std::optional<std::wstring> ReadLine(std::wstring_view file, bela::error_code &ec,
                                            uint64_t maxline = MaximumLineLength) {
  std::wstring line;
  if (ReadLine(file, line, ec, maxline)) {
    return std::make_optional(std::move(line));
  }
  return std::nullopt;
}
bool WriteTextU16LE(std::wstring_view text, std::wstring_view file, bela::error_code &ec);
bool WriteText(std::string_view text, std::wstring_view file, bela::error_code &ec);
bool WriteTextAtomic(std::string_view text, std::wstring_view file, bela::error_code &ec);
inline bool WriteText(std::wstring_view text, std::wstring_view file, bela::error_code &ec) {
  return WriteText(bela::ToNarrow(text), file, ec);
}
inline bool WriteText(std::u16string_view text, std::wstring_view file, bela::error_code &ec) {
  return WriteText(bela::ToNarrow(text), file, ec);
}

} // namespace bela::io

#endif