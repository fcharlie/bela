//
#ifndef BELA_BUFIO_HPP
#define BELA_BUFIO_HPP
#include "base.hpp"
#include <algorithm>

namespace bela::bufio {
constexpr int default_buffer_size = 4096;
// Fixed capacity size bufio.Reader implementation
template <int Size = default_buffer_size> class Reader {
public:
  Reader(HANDLE r) : fd(r) {}
  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;
  int Read(void *buffer, int len, bela::error_code &ec) {
    if (buffer == nullptr || len == 0) {
      return 0;
    }
    if (r == w) {
      if (len > cap) {
        // Large read, empty buffer.
        // Read directly into p to avoid copy.
        int rlen = 0;
        if (!fsread(buffer, len, rlen, ec)) {
          return -1;
        }
        return rlen;
      }
      r = 0;
      w = 0;
      int rlen = 0;
      if (!fsread(data, sizeof(data), rlen, ec)) {
        return -1;
      }
      if (rlen == 0) {
        return 0;
      }
      w += rlen;
    }
    auto n = (std::min)(w - r, len);
    memcpy(buffer, data + r, n);
    r += n;
    return len;
  }
  int ReadFull(void *buffer, int len, bela::error_code &ec) {
    int n = 0;
    auto p = reinterpret_cast<uint8_t *>(buffer);
    for (; n < len;) {
      auto nn = Read(p + n, len - n, ec);
      if (nn == -1) {
        return -1;
      }
      n += nn;
      if (n == 0) {
        break;
      }
    }
    return n;
  }

  constexpr int size() const { return Size; }

private:
  HANDLE fd;
  uint8_t data[Size];
  int w{0};
  int r{0};
  int cap{0};
  bool fsread(void *b, int len, int &rlen, bela::error_code &ec) {
    DWORD dwSize = {0};
    if (ReadFile(fd, b, static_cast<DWORD>(len), &dwSize, nullptr) != TRUE) {
      ec = bela::make_system_error_code(L"ReadFile: ");
      return false;
    }
    rlen = static_cast<int>(len);
    return true;
  }
};
} // namespace bela::bufio

#endif