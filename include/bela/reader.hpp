//
#ifndef BELA_READER_HPP
#define BELA_READER_HPP
#include "base.hpp"
#include "buffer.hpp"
#include <type_traits>

namespace bela {
using closer_t = std::add_pointer_t<BOOL WINAPI(HANDLE)>;
class Reader {
public:
  Reader() = default;
  Reader(HANDLE fd_) : fd(fd_) {}
  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;
  Reader(Reader &&other) { MoveFrom(std::move(other)); }
  Reader &operator=(Reader &&other) {
    MoveFrom(std::move(other));
    return *this;
  }
  Reader &Reset(HANDLE fd_) {
    Free();
    fd = fd_;
    return *this;
  }
  bool NewFile(std::wstring_view file, bela::error_code &ec) {
    if (fd != INVALID_HANDLE_VALUE) {
      ec = bela::make_error_code(L"The file has been opened, the function cannot be called repeatedly");
      return false;
    }
    fd = CreateFileW(file.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fd == INVALID_HANDLE_VALUE) {
      ec = bela::make_system_error_code();
      return false;
    }
    closer = CloseHandle;
    return true;
  }
  HANDLE FD() const { return fd; }
  bool PositionAt(uint64_t pos, bela::error_code &ec) const {
    auto li = *reinterpret_cast<LARGE_INTEGER *>(&pos);
    LARGE_INTEGER oli{0};
    if (SetFilePointerEx(fd, li, &oli, SEEK_SET) != TRUE) {
      ec = bela::make_error_code(L"SetFilePointerEx: ");
      return false;
    }
    return true;
  }
  bool PositionEnd(uint64_t pos, bela::error_code &ec) const {
    auto li = *reinterpret_cast<LARGE_INTEGER *>(&pos);
    LARGE_INTEGER oli{0};
    if (SetFilePointerEx(fd, li, &oli, SEEK_END) != TRUE) {
      ec = bela::make_error_code(L"SetFilePointerEx: ");
      return false;
    }
    return true;
  }
  bool Read(void *buffer, size_t len, size_t &outlen, bela::error_code &ec) const {
    DWORD dwSize = {0};
    if (ReadFile(fd, buffer, static_cast<DWORD>(len), &dwSize, nullptr) != TRUE) {
      ec = bela::make_system_error_code(L"ReadFile: ");
      return false;
    }
    outlen = static_cast<size_t>(len);
    return true;
  }
  bool ReadAt(bela::Buffer &b, size_t len, uint64_t pos, bela::error_code &ec) const {
    if (len > b.capacity()) {
      b.grow(bela::align_length(len));
    }
    if (!PositionAt(pos, ec)) {
      return false;
    }
    return Read(b.data(), len, b.size(), ec);
  }
  bool ReadAt(void *buffer, size_t len, uint64_t pos, size_t &outlen, bela::error_code &ec) {
    if (!PositionAt(pos, ec)) {
      return false;
    }
    return Read(buffer, len, outlen, ec);
  }
  bool Read(bela::Buffer &b, bela::error_code &ec) const { return Read(b.data(), b.capacity(), b.size(), ec); }
  bool Read(bela::Buffer &b, size_t len, bela::error_code &ec) const { return Read(b.data(), len, b.size(), ec); }
  bool ReadAt(bela::Buffer &b, uint64_t pos, bela::error_code &ec) const { return ReadAt(b, b.capacity(), pos, ec); }

private:
  HANDLE fd{INVALID_HANDLE_VALUE};
  closer_t closer{nullptr};
  void Free() {
    if (closer != nullptr && fd != INVALID_HANDLE_VALUE) {
      closer(fd);
    }
    closer = nullptr;
    fd = INVALID_HANDLE_VALUE;
  }
  void MoveFrom(Reader &&other) {
    Free();
    fd = other.fd;
    closer = other.closer;
    other.fd = INVALID_HANDLE_VALUE;
    other.closer = nullptr;
  }
};
} // namespace bela

#endif